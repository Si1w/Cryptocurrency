#include "client.h"

Client::Client(std::string id, const Server& server) : server(&server), id(id) {
    crypto::generate_key(public_key, private_key);
}

std::string Client::get_id() {
    return id;
}

std::string Client::get_publickey() const {
    return public_key;
}

double Client::get_wallet() const {
    return server->get_wallet(id);
}

std::string Client::sign(std::string txt) const{
    return crypto::signMessage(private_key, txt);
}

bool Client::transfer_money(std::string receiver, double value) const {
    // change value to 2 decimal places
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    std::string value_str = stream.str();
    std::string trx = id + "-" + receiver + "-" + value_str;
    std::string signature = crypto::signMessage(private_key, trx);
    return server->add_pending_trx(trx, signature);
}

size_t Client::generate_nonce() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, std::numeric_limits<size_t>::max());
    return dis(gen);
}


