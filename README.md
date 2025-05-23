# Beadandó

## OpenCL Kép Szűrési Projekt

### Projekt Leírása

Ez a projekt egy OpenCL-alapú képszűrő eszközt valósít meg, amely képes különböző szűrések (Sobel, Gauss, Medián) színes vagy szürkeárnyalatos képeken. A szűrés típusa, a bemenet, a kimenet és az eszköz(CPU/GPU) parancssori kapcsolókon keresztül állítható be.

### Fő Funkciók

- **RGB és szürkeárnyalatos képek támogatása**
- **Sobel szűrő**: autómatikusan átalakítja a képet szürkeárnyalatosra, majd éldetektálást végez
- **Gauss szűrő**: zajcsökkentő elmosás szines képeken is
- **Medián szűrő**: zajeltávolítás színes képeken is
- **Luma szűrő**: zajeltávolítás csak a fény erősség figyelembevételével
- **OpenCL GPU vagy CPU eszközválasztás**
- **Autómatikus mentés PNG formátumba**

### Használat

```bash
./image-filter \
	--input input.jpg \
	--output output.png \
	--filter sobel|gauss|median|luma \
	--device cpu|gpu
```

vagy ha az összesen filteren és képen szeretnéd futtatni akkor(gyökér mappában):

```
./run_all_filters.sh cpu|gpu
```

### Fájlstruktúra
```
Parallel-devices/
├── beadando/ # A beadandó projekt (korábbi src/)
│ ├── image_filter.c # Főprogram (host kód)
│ ├── filters.c # OpenCL eszközök, kernel hívás
│ ├── filters.h # Header
│ └── kernels.cl # OpenCL kernelkód
│
├── include/
│ ├── stb_image.h # Kép beolvasáshoz
│ └── stb_image_write.h # Kép mentéshez
│
├── images/
│ ├── input[number].jpeg # Tesztbemenet
│ └── outputs/ # Eredményképek
│   └── output[number]_[filter]_[device].png #Eredménykép
├── build/ # Fordított binárisok helye
│ └── (ide kerül a `beadando/image_filter` vagy más feladat binárisa)
│
├── vector_add/ # Órai projekt
├── rank/ # Órai projekt
├── info/ # Órai projekt
│ └── ...
│
├── Makefile # Általános makefile (moduláris)
├── run_all_filters.sh
├── README.md # A beadandó dokumentációja
└── measurements.csv # Mérési eredmények külön fájlban
```

### Mérési Eredmények

A következő táblázat néhány futtatás idejét mutatja különböző szűrőkkel és eszközökkel:

| Kép neve   | Méret (px) | Szűrő  | Eszköz | Futásidő (ms) |
| ---------- | ---------- | ------ | ------ | ------------- |
| input1.jpg | 1920x1080  | sobel  | gpu    | 8.4           |
| input1.jpg | 1920x1080  | gauss  | gpu    | 6.7           |
| input1.jpg | 1920x1080  | median | gpu    | 12.3          |
| input1.jpg | 1920x1080  | sobel  | cpu    | 24.1          |
| input1.jpg | 1920x1080  | gauss  | cpu    | 18.5          |
| input1.jpg | 1920x1080  | median | cpu    | 36.9          |

### measurements.csv példa

```
input_file,filter,device,width,height,elapsed_time(ms)
input1.jpeg,sobel,gpu,1024,1024,2.8670000000
input1.jpeg,gauss,gpu,1024,1024,3.0810000000
input1.jpeg,median,gpu,1024,1024,24.8590000000
input1.jpeg,luma,gpu,1024,1024,81.2860000000
input2.jpeg,sobel,gpu,4032,3024,7.4530000000
...
```

### Követelmények

- OpenCL SDK
- C fordító (pl. gcc vagy clang)
- stb_image.h és stb_image_write.h a `include/` könyvtárban
- cpu-n való futtatás esetén windows rendszer

### Fordítás

```bash
make FOLDER=image_filter
```

### Licensz

Ez a projekt MIT licensz alatt kerül megosztásra.

```text
MIT License

Copyright (c) 2025 Tóth Tamás

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
