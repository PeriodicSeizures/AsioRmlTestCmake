#include <asio.hpp>
#include <iostream>
#include <string>

std::string getStockPage(std::string const& ticker) {
    asio::ip::tcp::iostream stream;

    stream.connect("www.google.com", "http");
    stream << "GET /search?q=" << ticker << " HTTP/1.1\r\n";
    stream << "Host: www.google.com\r\n";
    stream << "Cache-Control: no-cache\r\n";
    // stream << "Content-Type: application/x-www-form-urlencoded\r\n\r\n";
    stream << "Connection: close\r\n\r\n" << std::flush;

    std::ostringstream os;
    os << stream.rdbuf();
    return os.str();
}

int main() {
    std::cout << getStockPage("$tsla");
}