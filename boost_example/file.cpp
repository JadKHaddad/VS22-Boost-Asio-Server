#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>



class Animal
{
public:
    Animal() {}
    void set_leg(int l) { legs = l; };
    void set_name(std::string s) { name = s; };
    void set_ismammal(bool b) { is_mammal = b; };
    void print();

private:
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, unsigned)
    {
        ar &BOOST_SERIALIZATION_NVP(legs) & BOOST_SERIALIZATION_NVP(is_mammal) & BOOST_SERIALIZATION_NVP(name);
    }

    int legs;
    bool is_mammal;
    std::string name;
};

void Animal::print()
{
    std::cout << name << " with " << legs << " legs is " << (is_mammal ? "" : "not ") << "a mammal" << std::endl;
}

void save_obj(const Animal &animal, std::stringstream &stream)
{
    MyOArchive oa{stream};
    oa << animal;
}

int main()
{
    std::stringstream stream;
    {
        Animal animal;
        animal.set_name("Horse");
        animal.set_leg(4);
        animal.set_ismammal(true);

        save_obj(animal, stream);
    }

    std::cout << "stream print: " << stream.str() << std::endl;
}