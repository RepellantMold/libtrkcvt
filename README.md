# s3m2stm

[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=plastic)](https://github.com/RichardLitt/standard-readme)

A command line tool to downgrade Scream Tracker 3 modules to Scream Tracker 2 (and Scream Tracker Music Interface Kit) modules, written in pure ANSI C/C89 (with 16-bit DOS compilers in mind).

## Table of Contents

- [Usage](#usage)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Usage

```sh
./s3m2stm input.s3m output.stm
```

> [!IMPORTANT]
> These points are important for a decent conversion.
> 
> - 4 channels preferably (it only converts the first four channels.)
> - Sample sizes cannot exceed 65535 (and no 16-bit samples.)
> - No panning or Adlib, Scream Tracker 2/Scream Tracker Music Interface Kit is mono.
> - Sample names are gonna be truncated heavily to 8.3 filenames, mainly for the purposes of being able to save it to disk.
> - You can only use effects A - J, with some caveats with the table shown below.
> 
> | Effect | Function in ST2/STMIK - in ST3/others | Notes/Quirks (Effect memory **does not exist** in Scream Tracker 2, so a parameter of 0 will act like a no-op for any effect that normally has it. (e.g. volume slide, portamento slides, vibrato, etc.)) |
> |---:|:---:|---|
> | Axx | Set speed - Set ticks per row | It has a scaling factor (TODO: handle it) alongside setting ticks per row. |
> | Bxx | Set next order - Position jump | It does not perform an immediate pattern break unlike most other trackers, so it's best to place it alongside a Pattern break. |
> | Cxx | Pattern break | The parameter is ignored which is identical behavior to NoiseTracker. |
> | Dxy | Volume slide | There's no fine slides, and y will take priority if both x and y are specified (which is backwards from how it's usually handled). |
> | Exx | Portamento down | This effect can cause the note period to underflow, and there are no fine/extra-fine slides. |
> | Fxx | Portamento up | This effect can cause the note period to overflow, ditto for fine/extra-fine slides. |
> | Gxx | Tone portamento |  |
> | Hxy | Vibrato | The depth is doubled compared to other trackers, this converter will attempt to adjust the depth accordingly. |
> | Ixy | Tremor | A parameter of 0 will be a very fast tremor, like old Scream Tracker 3. |
> | Jxy | Arpeggio | The effect behaves strangely since it tends to skip to y halfway through a row if x was 0, it's not commonly implemented this way however. |
> | Kxx | (No-op) | This can be entered into the editor but it does not do anything, **it also bugs out Scream Tracker 3 when importing so it's best to not use it!** |
> | Lxx | (No-op) | Ditto. |
> | Mxx | (No-op) | Ditto. |
> | Nxx | (No-op) | Ditto. |
> | Oxx | (No-op) | Ditto. |

## Maintainers

[@RepellantMold](https://github.com/RepellantMold)
[@cs127](https://github.com/cs127)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the
[standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

[ISC](LICENSE) © 2024 RepellantMold
