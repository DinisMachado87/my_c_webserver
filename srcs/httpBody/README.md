Body paths:
- Chunked in: ChunkDecoder (de-frames, gathers payload spans)
- Chunked out: ChunkEncoder (frames as hex\r\n...\r\n)
- Unchunked: IOBuffer directly — no framing, Writer::Kind picks
  syscall. Content-Length stop condition owned by caller (TBD).

# httpBody

HTTP chunked Transfer-Encoding, both directions. Depends on `srcs/buffers`.
Makes no syscalls directly — I/O goes through the base buffer's Writer.

Body not chunked uses IOBuffer directly — no framing, Writer::Kind picks syscall. Content-Length stop condition owned by caller
`ChunkEncoder` — reads plain, writes chunked.
`ChunkDecoder` — chunked in, plain out.
Both `: public IOBuffer`, both override `writeOut()`, both take their
`Writer::Kind` at construction. `ChunkDecodeError` on malformed framing.

## Zero-copy

Payload is read into Segments once, never copied.

Encoder interleaves header spans (`_sizeHeader[]`, member array), payload
spans (into Segments), and a static `"\r\n"`. Decoder parses framing in
place and gathers only payload spans, skipping size lines and CRLFs where
they sit. One `sendmsg` each.

## Partial sends

`writeOut()` runs on EPOLLOUT. Short write leaves the batch half-drained;
`advance(n)` trims and offsets, next call resumes the same batch.

Do not rebuild on resume. Do not return Segments to the pool until
`_batch.done()` — the iovecs still point into them.

## Decoder cursor

`_cursor`/`_pos` walk the chain independently of `_readEnd`. Segments
behind the cursor are poppable; the cursor's own segment is not.

`discardConsumed()` must not run with a NULL cursor. NULL means the parser
walked off the end mid-chunk — popping then destroys the segment `_pos`
refers to, and the next `prepareBatch()` re-parses from the front with a
stale offset. Silent corruption, not a crash.
