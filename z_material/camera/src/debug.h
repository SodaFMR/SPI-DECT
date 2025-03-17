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

#ifndef DECT2020_DEBUG_H
#define DECT2020_DEBUG_H

#include <stdint.h>
// #include <fstream>

//extern "C"{
//#include <debug.h>
//}

#define DECT2020_SUCCESS    0
#define DECT2020_ERROR      -1



// void file_binary_representation_msb_first(const char* filename, const uint8_t* in, uint32_t len_bytes);
// void file_binary_representation_lsb_first(const char* filename, const uint8_t* in, uint32_t len_bytes);

// template<typename T>
// void file_binary_representation_unpacked(const char* filename, const T* in, uint32_t len, bool convert_to_1_0){
//     std::ofstream f(filename, std::ios::trunc);
//     for(uint32_t i=0; i<len; i++){
//         if(convert_to_1_0){
//             if(in[i] > 0)
//                 f << (int) 1 << std::endl;
//             else
//                 f << (int) 0 << std::endl;
//         }
//         else{
//             f << (int) in[i] << std::endl;
//         }
//     }
//     f.close();
// }


#endif
