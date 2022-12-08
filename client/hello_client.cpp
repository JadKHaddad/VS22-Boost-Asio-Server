#include <iostream>
#include <boost/asio.hpp>

using namespace boost;

int main( int argc, char** argv )
{
    const size_t   size = 13;
    unsigned short server_port = 1259;
    std::string    server_ip{ "127.0.0.1" };
    
    if( argc > 1 ) { server_ip = argv[1]; }
    if( argc > 2 ) { server_port = std::atoi( argv[2] ); }
    
      try
    {
        asio::io_service        io_service{};
        asio::ip::tcp::socket   socket{ io_service };
        asio::ip::tcp::endpoint endpoint{
            asio::ip::address::from_string( server_ip ),
            server_port };

        std::cout << "connecting to " << endpoint << "..." << std::endl;
        socket.connect( endpoint );

        asio::write( socket, asio::buffer( "Hello server!" ) );
        
        char read_buf[size];
        asio::read( socket, asio::buffer( read_buf, size ) ); 
        std::cout << std::string{ read_buf, size } << std::endl;
    }
    
    catch( const system::system_error& err )
    {
        std::cerr << "[error] " << err.what() << std::endl;
    } }
