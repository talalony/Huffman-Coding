# Huffman Coding in C

This project implements Huffman coding in C for compressing and decompressing text files. 
Huffman coding is a lossless data compression algorithm that efficiently encodes data by assigning shorter codes to more frequent characters and longer codes to less frequent characters.

## Usage
- Build by running:
```
git clone https://github.com/talalony/Huffman-Coding.git
cd Huffman-Coding
gcc Huffman.c -o huffman
```
- Compression
```
./huffman -c input
```
- Decompression
```
./huffman -d input.huf
```
