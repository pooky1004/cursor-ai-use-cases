use std::ops::RangeInclusive;
use std::thread;
use std::mem::size_of;
use std::net::Ipv4Addr;
use std::net::UdpSocket;
use std::time::{Duration, SystemTime};

use rand::Rng;
use num_format::{Locale, ToFormattedString};

use clap::{ArgAction, Command, arg, value_parser};
use clap::builder::PossibleValue;

// Ethernet + IPv4 + UDP
const HEADER_SIZE: usize = {
    let ethernet = 14;
    let ipv4 = 20;
    let udp = 8;
    ethernet + ipv4 + udp
};
const MTU: usize = 1500;

const TEST_HEADER_SIZE: usize = {
    size_of::<u32>() + // packet_id
    size_of::<u16>() // packet_len
};

struct Statistics {
    sent_packets: usize,
    sent_bytes: usize,
    received_packets: usize,
    received_bytes: usize,
    errored_packets: usize,
}

static mut STATS: Statistics = Statistics {
    sent_packets: 0,
    sent_bytes: 0,
    received_packets: 0,
    received_bytes: 0,
    errored_packets: 0,
};

const RECEIVER_COOLDOWN: u64 = 2;
const BULK_SIZE: usize = 1_000;

fn main() {
    let matched_command = Command::new("udp-throughput")
        .arg(arg!(<target> "Target IP address")
            .required(true)
            .value_parser(value_parser!(Ipv4Addr)))
        .arg(arg!(-p --port <port> "Target port")
            .default_value("7")
            .value_parser(value_parser!(u16)))
        .arg(arg!(-s --size <size> "Size of the packet")
            .long_help("You can specify minsize and maxsize by give this value twice")
            .default_value("64")
            .action(ArgAction::Append)
            .value_parser(value_parser!(usize)))
        .arg(arg!(-d --duration <duration> "Duration of the test")
            .default_value("10")
            .value_parser(value_parser!(u64)))
        .arg(arg!(-P --pattern <pattern> "Pattern to send")
            .value_parser([
                PossibleValue::new("zero").help("Zero fill"),
                PossibleValue::new("count").help("Incrementing bytes"),
                PossibleValue::new("hash").help("Hashed bytes sequence"),
            ])
            .default_value("count"))
        .arg_required_else_help(true)
        .get_matches();

    let target: Ipv4Addr = *matched_command.get_one("target").unwrap();
    let port: u16 = *matched_command.get_one("port").unwrap();
    let sizes: Vec<usize> = matched_command.get_many("size").unwrap().copied().collect();
    let duration = *matched_command.get_one("duration").unwrap();
    let pattern: String = matched_command.get_one::<String>("pattern").unwrap().to_owned();

    let (minsize, maxsize) = match sizes.len() {
        1 => (sizes[0], sizes[0]),
        2 => (sizes[0], sizes[1]),
        _ => panic!("Invalid number of sizes"),
    };
    if minsize < HEADER_SIZE + TEST_HEADER_SIZE {
        eprintln!("Minimum size must >= {}", HEADER_SIZE + TEST_HEADER_SIZE);
        return;
    }
    if maxsize > MTU {
        eprintln!("Maximum size must <= {}", MTU);
        return;
    }
    if minsize > maxsize {
        eprintln!("Minimum size must be less than or equal to maximum size");
        return;
    }

    println!("Using configuration: target={}, port={}, minsize={}, maxsize={}, duration={}, pattern={}", target, port, minsize, maxsize, duration, pattern);

    let sock_tx = UdpSocket::bind("0.0.0.0:0").expect("Failed to bind UDP socket");
    let sock_rx = sock_tx.try_clone().expect("Failed to clone UDP socket");
    sock_tx.set_nonblocking(true).expect("Failed to set non-blocking mode");
    sock_rx.set_nonblocking(true).expect("Failed to set non-blocking mode");
    // sock_rx.set_read_timeout(Some(Duration::from_secs(1)))
    //     .expect("Failed to set read timeout");

    let sender = {
        let pattern = pattern.clone();
        thread::spawn(move || {
            send_worker(&sock_tx, target, port, minsize..=maxsize, duration, &pattern)
        })
    };
    let receiver = {
        let pattern = pattern.clone();
        thread::spawn(move || {
            recv_worker(&sock_rx, target, port, duration, &pattern);
        })
    };

    thread::sleep(Duration::from_secs(duration as u64));

    // Join threads
    sender.join().expect("Failed to join sender thread");
    receiver.join().expect("Failed to join receiver thread");

    // Print statistics
    #[allow(static_mut_refs)]
    let stats = unsafe { &STATS };
    let sent_pps = stats.sent_packets / duration as usize;
    let sent_bps = stats.sent_bytes / duration as usize * 8;
    let received_pps = stats.received_packets / duration as usize;
    let received_bps = stats.received_bytes / duration as usize * 8;
    let missed_packets = stats.sent_packets - stats.received_packets - stats.errored_packets;
    let missed_ratio = missed_packets as f64 / stats.sent_packets as f64;
    let errored_ratio = stats.errored_packets as f64 / stats.sent_packets as f64;

    println!(
        "Sent: {} packets ({} bytes), {} pps, {} bps",
        stats.sent_packets.to_formatted_string(&Locale::en),
        stats.sent_bytes.to_formatted_string(&Locale::en),
        sent_pps.to_formatted_string(&Locale::en),
        sent_bps.to_formatted_string(&Locale::en),
        );
    println!(
        "Received: {} packets ({} bytes), {} pps, {} bps",
        stats.received_packets.to_formatted_string(&Locale::en),
        stats.received_bytes.to_formatted_string(&Locale::en),
        received_pps.to_formatted_string(&Locale::en),
        received_bps.to_formatted_string(&Locale::en),
        );
    println!(
        "Missed: {} packets({:.2} %)",
        missed_packets.to_formatted_string(&Locale::en),
        missed_ratio * 100.0);
    println!(
        "Errored: {} packets ({:.2} %)",
        stats.errored_packets.to_formatted_string(&Locale::en),
        errored_ratio * 100.0);
}

fn send_worker(sock: &UdpSocket, target: Ipv4Addr, port: u16, range: RangeInclusive<usize>, duration: u64, pattern: &str) {
    let duration = Duration::from_secs(duration);
    let start = SystemTime::now();

    while start.elapsed().unwrap() < duration {
        for _ in 0..BULK_SIZE {
            let total_size = rand::thread_rng().gen_range(range.clone());
            let udp_size = total_size - HEADER_SIZE;
            let payload_size = udp_size - TEST_HEADER_SIZE;
            let mut buffer = vec![0u8; udp_size];
            let mut payload_buffer = &mut buffer[TEST_HEADER_SIZE..];

            let packet_id = unsafe { STATS.sent_packets } as u32;
            match pattern {
                "zero" => {
                    payload_buffer.iter_mut().for_each(|byte| *byte = 0);
                }
                "count" => {
                    payload_buffer.iter_mut().for_each(|byte| *byte = packet_id as u8);
                }
                "hash" => {
                    for (i, byte) in payload_buffer.iter_mut().enumerate() {
                        *byte = packet_id.wrapping_add(i as u32).wrapping_add(0x55) as u8;
                    }
                }
                _ => unreachable!(),
            }

            buffer[0..4].copy_from_slice(&packet_id.to_be_bytes());
            buffer[4..6].copy_from_slice(&(udp_size as u16).to_be_bytes());

            match sock.send_to(&buffer, (target, port)) {
                Ok(_) => {
                    unsafe {
                        STATS.sent_packets += 1;
                        STATS.sent_bytes += total_size;
                    }
                }
                Err(_e) => {
                    // eprintln!("Failed to send packet: {}", e);
                }
            }
        }
    }

    eprintln!("Sender finished");
}

fn recv_worker(sock: &UdpSocket, target: Ipv4Addr, port: u16, duration: u64, pattern: &str) {
    let duration = Duration::from_secs(duration + RECEIVER_COOLDOWN);
    let mut buffer = vec![0u8; MTU];

    let start = SystemTime::now();
    while start.elapsed().unwrap() < duration {
        let received_size = match sock.recv_from(&mut buffer) {
            Ok((len, addr)) => {
                if addr.ip() != target || addr.port() != port {
                    continue;
                }
                len
            }
            Err(_e) => {
                // eprintln!("Failed to receive packet: {}", e);
                continue;
            }
        };

        let packet_id = u32::from_be_bytes(buffer[0..4].try_into().unwrap());
        let packet_size = u16::from_be_bytes(buffer[4..6].try_into().unwrap()) as usize;

        let payload = &buffer[TEST_HEADER_SIZE..received_size];

        let is_valid = received_size == packet_size && match pattern {
            "zero" => payload.iter().all(|byte| *byte == 0),
            "count" => payload.iter().all(|byte| *byte == packet_id as u8),
            "hash" => payload.iter().enumerate().all(|(i, byte)| *byte == packet_id.wrapping_add(i as u32).wrapping_add(0x55) as u8),
            _ => unreachable!(),
        };

        if !is_valid {
            unsafe {
                STATS.errored_packets += 1;
            }
            eprintln!("Received packet is not the same as sent packet");
        } else {
            unsafe {
                STATS.received_packets += 1;
                STATS.received_bytes += received_size + HEADER_SIZE;
            }
        }
    }

    eprintln!("Receiver finished");
}
