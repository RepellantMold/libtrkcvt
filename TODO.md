### potential ideas

- give either an optional verbose option or a quiet option to reduce the amount of output
- output both version 1.0 and 1.1 STX files based on command line option (default to 1.1?)


### todo

- [ ] add a channel remap system if there's muted channels, it would prevent someone from muting the first 4 channels and breaking the conversion
- [ ] work on stx conversion (and switch based on file extension of the output file)

### in progress

- [ ] work on pattern conversion
- [ ] sample header conversion (the parapointers at the end of the header; how do we handle them?)

### complete

- [x] song header conversion
- [x] work on sign conversion with the samples
- [x] have the order conversion discard markers