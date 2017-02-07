# range_coder

This project is targeted at developing a range coder with up to 1GB/s compression/decompression rate on a modern desktop computer (2017)
and more on a high-end server, ie. able to decompress an entire 1TB SSD drive in 16Mins and compress in under an hour.

We are experimenting with using a hybrid of a context sensitive markov model range coder (as in LZMA) and some variation
on a BWT (Burrows Wheeler Transform). We are not using an adaptive model as in LZMA and this gives us some considerable
performance improvement. In addition to this Gzip, Bzip2 and LZMA are stubbornly single threaded and LZMA uses a highly
complex coding scheme which is hard to optimize.

Our library is a modern C++ header only one suitable for single module compilation (ie. sub ten second builds for large projects).

By comparison, 

(http://tukaani.org/lzma/benchmarks.html)

Uncompressed size: 212664320 bytes (203 MB)
Maximum compression (9)

Compression:

| codec | gzip | bzip2 | lzmash | lzmash -e | our goal |
| --- |  --- |  --- |  --- |  --- |  --- |
| final size | 78768334 | 72223858 | 54068819 | 53769958 | ~60000000 |
| rate (MB/s) | 3.0 | 1.3 | 0.27 | 0.15 | ~250.0 |

Decompression:

| codec | gzip | bzip2 | lzmash | lzmash -e | our goal |
| --- |  --- |  --- |  --- |  --- |  --- |
| rate (MB/s) | 65 | 5.3 | 20 |20 | ~1000.0 |

