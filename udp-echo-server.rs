use std::net::UdpSocket;

fn main() {
    let socket = UdpSocket::bind("0.0.0.0:7").expect("Failed to bind the socket");

    let mut buf = [0; 1500];
    loop {
        let (amt, src) = socket.recv_from(&mut buf).expect("Failed to receive data");

        socket.send_to(&buf[..amt], &src).expect("Failed to send data back");
    }
}
