### 4. `STDLIB.md`
```markdown
 Standard Library Substitutions

* **Audio Parsing (`libsndfile`)** -> Hand-rolled a RIFF/WAVE header parser using `fread`, manually reading byte chunks and shifting bits (`readLE32`, `readLE16`) to avoid endianness issues across architectures.
* **Signal Processing (`FFTW`)** -> Implemented a custom Short-Time Fourier Transform (STFT) inside `shortFourier()` relying entirely on standard C `<math.h>` primitives (`sinf`, `cosf`, `sqrtf`).
* **Note Conversion (`libaubio`)** -> Calculated acoustic half-steps from a base frequency manually using `log2f` and `roundf`.
* **Storage Engine (`SQLite` / `Redis`)** -> Built a custom binary append-only database (`audio.txt`). Serialization and deserialization are handled directly via `fwrite`/`fread` mapping to C structs (`audioInfo_s`), paired with an in-memory hash table mapping buckets to byte offsets.
