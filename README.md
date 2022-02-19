# Documentation

[Link to auto-generated documentation](https://pcsea-2022.pages.ensimag.fr/x86-aubeadri-laclautv-sarrya-elhajlah/psys-base/)
(updated every commit)

## Generating and creating docs

We use [Doxygen](https://www.doxygen.nl/index.html).

```shell
sudo apt install -y doxygen graphviz
# runs doxygen
make doc 
```

Doc will be in `public/index.html`.

For your function/struct comment to be picked up by Doxygen, start it with a slash and two stars: `/**`. For example:

```shell
/**
 * Creates a new process.
 * @param pt_func Pointer to the address that the process should begin
 * executing from.
 * @param ssize Stack size guantreed to the calling process.
 * @param prio Priority of this process. A higher priority will mean
 * this process will be given more CPU time.
 * @param name Name of this process.
 * @param arg An argument of the process.
 * @return The pid of the process, or -1 if either the arguments were
 * incorrect or there is not enough space to allocte a process.
 */
int start(int (*pt_func)(void *), unsigned long ssize, int prio,
	  const char *name, void *arg);
```

