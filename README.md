 ## Building
 
 g++ compiler version 15 or later is required. As embed support in c++ mode is required, no other compilers are supported.
 
 Clone the fchess repository.
```bash
$ git https://github.com/bftjoe/fchess.git
$ cd fchess
```
Download the latest neural network from this [repository](https://github.com/PGG106/Alexandria-networks/releases) and save it in the source root as `nn.net`.
```bash
$ make
$ ./fchess
```
 ## How to use the engine

The Universal Chess Interface (UCI) is a standard protocol used to communicate with
a chess engine, and is the recommended way to do so for typical graphical user interfaces
(GUI) or chess tools. Alexandria implements the majority of its options as described
in [the UCI protocol](https://www.shredderchess.com/download/div/uci.zip).

## Acknowledgements

This project is by bftjoe and based on [Alexandria](https://github.com/PGG106/Alexandria/tree/master) by Zuppa (PGG106). 

Please see the Alex readme and the Contributors file for more credits.

* fchess uses neural networks trained on [data provided by the Leela Chess Zero
project](https://storage.lczero.org/files/training_data), which is made available under the [Open Database License](https://opendatacommons.org/licenses/odbl/odbl-10.txt) (ODbL).
