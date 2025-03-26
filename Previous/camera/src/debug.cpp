/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Maxim Penner <maxim.penner@ikt.uni-hannover.de>
 */

#include <string>
#include <bitset>
#include <algorithm>
#include <include/utils/debug.h>

static void write_bits_of_byte_one_per_line(std::ofstream &f, std::string s){
    for(uint32_t j=0; j<8; j++){
        f << s[j] << std::endl;
    }
}

namespace dect2020 {

void file_binary_representation_msb_first(const char* filename, const uint8_t* in, uint32_t len_bytes){

    std::ofstream f(filename, std::ios::trunc);
    for(uint32_t i=0; i<len_bytes; i++){
        std::bitset<8> b(in[i]);
        std::string s = b.to_string();
        write_bits_of_byte_one_per_line(f, s);
    }
    f.close();
}

void file_binary_representation_lsb_first(const char* filename, const uint8_t* in, uint32_t len_bytes){
    std::ofstream f(filename, std::ios::trunc);
    for(uint32_t i=0; i<len_bytes; i++){
        std::bitset<8> b(in[i]);
        std::string s = b.to_string();
        std::reverse(s.begin(), s.end());
        write_bits_of_byte_one_per_line(f, s);
    }
    f.close();
}

}
