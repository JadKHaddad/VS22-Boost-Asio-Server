#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/asio.hpp>
#include <iostream>

//boost::serialization
struct blank
{
    int m_id;
    std::string m_message;

    template<typename archive> void serialize(archive& ar, const unsigned /*version*/) {
        ar & m_id;
        ar & m_message;
    }
};


int main() {
boost::asio::streambuf buf;
blank info;
info.m_id = 1;
info.m_message = "Rasul";

{
    std::ostream os(&buf);
    boost::archive::binary_oarchive out_archive(os);
    out_archive << info;
}
}