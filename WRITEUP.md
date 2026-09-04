# Vibe-Matching Audio in Pure C: Surviving the Zero Dependency Hackathon

I have a habit of getting hooked on a specific song and looping it for hours because I simply cannot find another track with the exact same musical "vibe." For the Zero Dependency Hackathon, I decided to solve my own problem: I built a song recommender that analyzes the top 10 musical note amplitudes to find tracks with similar tonal shifts. 

Because I like a challenge (and because this is only my third programming project ever), I chose **Track F (Wildcard)** and wrote the entire thing in **C**, using only `libc`. No external audio parsing, no math libraries, no database engines. 

Here is how I survived 72 hours in raw C, what I built, and the pointer arithmetic that almost broke me.

### What I Reimplemented (and the Packages I Made Unnecessary)
To analyze audio files without dependencies, I had to replace three massive pillars of a normal audio processing stack:

1. **`libsndfile` -> Hand-rolled `.wav` parser:** I wrote a custom parser to strip headers and extract the raw PCM audio data directly.
2. **`fftw3` -> Custom Discrete Fourier Transform (DFT):** I knew the math behind Fourier transforms, so I wrote my own DFT algorithm to extract the frequency amplitudes from the audio signals.
3. **`uthash` & SQLite -> Custom Hash Map & Binary DB:** To store and compare the audio profiles, I built an append-only binary database and wrote a hash map from scratch to manage the indexing. 

### The Edge Case That Ate an Afternoon: The Pointer Math Nightmare
When you write everything from scratch in C, you don't just fight logic bugs; you fight memory. My afternoon-killer was a persistent, silent crash caused by mixing up row-major and column-major array logic during pointer arithmetic.

**corrected version, cause I don't have any images of the wrong one**
<img width="1643" height="865" alt="image" src="https://github.com/user-attachments/assets/3fc2c1fe-51ef-4fea-bfb4-03b2ab7e8089" />

Because the program would just quietly fail, I had to turn off massive chunks of the project, replace them with dummy functions, and litter the code with `printf` statements just to trace the execution cycles. The logs eventually revealed a pattern: if I tried to add the exact same file to the database, the program crashed at the exact same cycle count every time. 

It turned out I had completely messed up the arguments requiring pointer arithmetic when passing my multidimensional arrays between functions. Once I finally spotted the root cause in one function, I had to hunt down the identical logic error across the entire codebase. (Full disclosure: one of these bugs still survived into the final submission!).

### What the Standard Library Made Painful
In C, the standard library is basically just files, sockets, and standard output. The two biggest pain points were:

1. **Zero Data Structures:** I had never used a hash map before this hackathon. Because `libc` doesn't include one, I had to spend precious hackathon hours actually *learning* how hash functions and hash maps work theoretically, and then implement one entirely from scratch just to store my data. 
2. **O(N²) Math Limits:** Writing the math for the Discrete Fourier Transform was straightforward, but without an external library like `fftw3`, my implementation was a naive $O(N^2)$ algorithm. It is incredibly slow. A Fast Fourier Transform (FFT) at $O(N \log N)$ is vastly superior, but building a highly optimized FFT from scratch within a 72-hour window on top of a database and parser was simply out of scope. 

### The Takeaway
I bit off way more than I could chew. By the second-to-last day, I realized the scope was too massive for 72 hours and had to brutally cut features to ship a working (doesn't work perfectly) artifact. 

But the zero-dependency constraint taught me a valuable lesson. It proved that while doing complex DSP math without a library will tank your performance, everyday tasks like file I/O, writing a database, and parsing binary headers are entirely doable with nothing but `libc`. 

The hackathon version is incomplete, but I am actively building out the full, optimized version (with a proper FFT!) over at [SequenceSync- on my GitHub](https://github.com/aplosaletheia/SequenceSync-). 

**Tag: Hackathon Raptors**
