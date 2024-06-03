#include "ClientConn.h"

Client::Client(asio::io_context& io_context, std::shared_ptr<DBConn> _db_connection, const std::string& server_address, unsigned short server_port)
	:io_context_(io_context), db_(_db_connection),
    socket_(io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
	server_endpoint_(asio::ip::address::from_string(server_address), server_port) {

    std::cout << "Client to " << server_address << ":" << server_port << " | Self: " << socket_.local_endpoint() << std::endl;
}

Client::~Client(){
}

bool Client::sendTransaction(const Transaction& tx) {
    asio::error_code error;

    //tx.printTx();

    std::array<uint8_t, 125> txBuffer = tx.Serialize();
    constexpr size_t bufferSize = txBuffer.size() + sizeof(TX_PROPAGATION);
    std::array<uint8_t, bufferSize> propagationBuffer;

    memcpy(propagationBuffer.data(), TX_PROPAGATION, sizeof(TX_PROPAGATION));
    memcpy(propagationBuffer.data() + sizeof(TX_PROPAGATION), txBuffer.data(), txBuffer.size());

    socket_.send_to(asio::buffer(propagationBuffer), server_endpoint_, 0, error);
    if (!error) {
        return true;
    }
    else {
        return false;        
    }
}
