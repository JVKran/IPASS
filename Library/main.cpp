#include "hwlib.hpp"
#include "TEA5767.hpp"
#include "KY040.hpp"
#include "A24C256.hpp"

int main( void ){
    namespace target = hwlib::target; 
    
    auto scl = target::pin_oc( target::pins::d8 );
    auto sda = target::pin_oc( target::pins::d9 );
	auto i2c_bus = hwlib::i2c_bus_bit_banged_scl_sda(scl, sda);

	auto memory = A24C256(i2c_bus);

	memory.writeByte(0, 'z');
	hwlib::cout << memory.readByte(0) << hwlib::endl;

}