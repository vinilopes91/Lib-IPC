# IPC-Lib

Biblioteca de rotinas para comunicação entre processos *(IPC)* locais *(LPC)* por meio de operações send/receive.

As características da biblioteca IPC-Lib:

- A biblioteca deve dar suporte aos modos de comunicação síncrona e assíncrona.
- As operações sendS() e receiveS() implementam a comunicação síncrona, isto é, quando um processo invoca sendS() este tem a sua execução bloqueada até que outro processo execute a sua contraparte, ou seja, receiveS(). O mesmo ocorre com a receiveS(), ou seja, quando ela é chamada o processo chamador fica bloqueado até que a sendS() correspondente seja executada.
- As operações sendA() e receiveA() implementam a comunicação assíncrona, isto é, quando um processo chama sendA() este não tem a sua execução  loqueada. Neste caso a sendA() retorna sem bloquear o processo. O mesmo ocorre com a receiveA(), sendo que se esta for chamada e ainda não tenha dados para serem recebidos, a rotina retorna -1 indicando que não há dados, sem o bloqueio do processo.
- Em ambos os modos de operação, síncrono/assíncrono, a comunicação entre sendS/sendA e receiveS/receiveA se dá por meio do mecanismo de memória compartilhada (shared memóry) que deve ser usado pela biblioteca.
- A biblioteca IPC-Lib deve ser projetada para ser usada tanto por aplicações singlethreaded quanto multithreaded.

---

### Makefile

Para compilar os arquivos e gerar o executavel **main** na raiz do projeto.
```bash
make
```

Para remover os arquivos objetos (*.o) que estão dentro da pasta *src/*
```bash
make clean
```

---

Links uteis:
[Shared libraries with GCC on Linux](https://www.cprogramming.com/tutorial/shared-libraries-linux-gcc.html)
