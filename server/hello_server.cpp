#include <iostream>
#include <boost/asio.hpp>

using namespace boost;

int main(int argc, char **argv)
{
    const size_t size = 13;
    unsigned short port = 1259;

    if (argc > 1)
    {
        port = std::atoi(argv[1]);
    }

    try
    {
        asio::io_service io_service{};
        asio::ip::tcp::socket socket{io_service};
        asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), port};
        asio::ip::tcp::acceptor acceptor{io_service, endpoint};

        while (true)
        {
            acceptor.accept(socket);
            std::cout << "client connected: " << socket.remote_endpoint()
                      << std::endl;

            char read_buf[size];
            asio::read(socket, asio::buffer(read_buf, size));
            std::cout << std::string{read_buf, size} << std::endl;

            asio::write(socket, asio::buffer("Hello client!"));

            socket.close();
        }
    }
    catch (const system::system_error &err)
    {
        std::cerr << "[error] " << err.what() << std::endl;
    }
}
