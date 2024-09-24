#include "server.h"

std::vector<std::string> pending_trxs;

Server::Server() {}

std::shared_ptr<Client> Server::add_client(std::string id) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> e(1000, 9999);
    for (const auto& c : clients) {
        if (c.first->get_id() == id) {
            id += std::to_string(e(gen));
        }
    }
    auto cli = std::make_shared<Client>(id, *this);
    clients[cli] = 5.00;
    return cli;
}

std::shared_ptr<Client> Server::get_client(std::string id) const {
    for (const auto& c : clients) {
        if (c.first->get_id() == id) {
            return c.first;
        }
    }
    return nullptr;
}

double Server::get_wallet(std::string id) const {
    for (const auto& c : clients) {
        if (c.first->get_id() == id) {
            return c.second;
        }
    }
    return 0;
}

bool Server::parse_trx(std::string trx, std::string& sender, std::string& receiver, double& value) {
    std::regex trx_pattern(R"(^([A-Za-z]+)-([A-Za-z]+)-([0-9]+(\.[0-9]{1,2})?)$)");
    std::smatch match;
    if (std::regex_match(trx, match, trx_pattern)) {
        sender = match.str(1);
        receiver = match.str(2);
        value = std::stod(match.str(3));
        return true;
    }else {
        throw std::runtime_error("Invalid transaction format");
    }
    return false;
}

bool Server::add_pending_trx(std::string trx, std::string signature) const {
    std::string sender, receiver;
    double value;
    try {
        parse_trx(trx, sender, receiver, value);
    }catch (const std::runtime_error& e) {
        return false;
    }
    auto receiver_id = get_client(receiver);
    auto sender_id = get_client(sender);

    bool authentic = crypto::verifySignature(sender_id->get_publickey(), trx, signature);
    if (authentic && value <= get_wallet(sender)) {
        pending_trxs.push_back(trx);
        return true;
    }
    return false;
}

size_t Server::mine() {
    std::string mempool{};
    for (const auto& trx : pending_trxs) {
        mempool += trx;
    }
    size_t nonce = 0;
    bool flag = false; //unmined transactions
    while (!flag) {
        for (auto& c : clients) {
            nonce += c.first->generate_nonce();
            if (crypto::sha256(mempool + std::to_string(nonce)).substr(0, 10).find("000") != std::string::npos) {
                flag = true;
                c.second += 6.25; // mined
                for (const auto& trx : pending_trxs) {
                    std::string sender, receiver;
                    double value;
                    parse_trx(trx, sender, receiver, value);
                    clients[get_client(sender)] -= value;
                    clients[get_client(receiver)] += value;
                }
                pending_trxs.clear();
                return nonce;
            }
        }
    }
    return nonce;
}

void show_wallets(const Server& server) {
    // use a friend function to access the private member variable clients
    std::cout << std::string(20, '*') << std::endl;
    for(const auto& client: server.clients)
        std::cout << client.first->get_id() <<  " : "  << client.second << std::endl;
    std::cout << std::string(20, '*') << std::endl;
}