#include "ServerConn.h"

Server::Server(asio::io_context& io_context, std::shared_ptr<DBConn> _db_connection)
    :io_context_(io_context), socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 6001)),
    db_(_db_connection){

    std::cout << "Server instantiated on " << socket_.local_endpoint() << "\n";
}


void Server::start() {
    if (!socket_.is_open()) {
        std::cerr << "Socket is not open. Unable to start server." << std::endl;
        return;
    }

    receiveMessage();
}

Server::~Server(){
    socket_.close();
}

void Server::receiveMessage() {
    socket_.async_receive_from(asio::buffer(recv_buffer_), sender_endpoint_, // use sender endpoint for response on blockchain data
        [this](const std::error_code& error, std::size_t bytes_received) {
            if (!error) {
                handleMessage(bytes_received);
                receiveMessage(); // Continue listening for incoming messages
            }
            else {
                std::cerr << "Error receiving message: " << error.message() << std::endl;
            }
        }
    );
}

void Server::handleMessage(std::size_t bytes_received) {
    // Handle the message here
    std::string receivedData(recv_buffer_.begin(), recv_buffer_.begin() + bytes_received);
    char messageType[3];

    std::copy_n(receivedData.begin(), 2, messageType);

    // Ensure null-termination
    messageType[2] = '\0';

    std::cout << "Received message from " << sender_endpoint_.address().to_string()
        << ":" << sender_endpoint_.port() << " (Received Bytes: " << bytes_received << ")" << std::endl;
    std::cout << "Message Type: " << messageType << " - ";

    if (!std::strcmp(messageType, TX_PROPAGATION)) {

        std::array<uint8_t, 125> txBuffer;
        std::copy(recv_buffer_.begin() + 3, recv_buffer_.begin() + 3 + txBuffer.size(), txBuffer.begin());

        Transaction receivedTx(txBuffer);

        ServerOnTxPropagation(receivedTx);
        
        

    }
    else if (std::strcmp(messageType, BLOCK_PROPAGATION) == 0) {
        std::cout << "Block propagation\n";
    }
    else if (std::strcmp(messageType, SHARE_BLOCKCHAIN_DATA) == 0) {
        std::cout << "Share blockchain data\n";
    }
    else if (std::strcmp(messageType, PEER_C) == 0) {
        std::cout << "Peer connection\n";
    }
    else {
        std::cout << "Unknown message type\n";
    }
}

// --- Server handles ---
void Server::ServerOnTxPropagation(const Transaction tx) {
    if (mempool.size() < 3) {

        mempool.emplace_back(tx);
    }
    else {
        Block newBlock()
        // try creating a block and propogating it.
    }

    // add receivedTx to mempool?
    // when mempool reaches a certain amount or a cenrtain amount of time has passed -> create a block;

    /*

    */
}