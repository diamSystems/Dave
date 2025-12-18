## The Book of Dave (as preserved by **diam Systems Ltd.**)

1. **Hearken, classmates and compatriots.** We, the engineers of diam Systems
   Ltd., present unto you Dave: a modern operating system and kernel wrought in
   the spirit of Dennis and Ken’s venerable Unix, yet refreshed for the year of
   our silicon 2026.

2. **Dave walketh a familiar path**—processes, pipes, and plain-spoken system
   calls—but we have dusted every stone. Scheduler, linker, and structured I/O
   now speak the idioms of our age, that shells might trade JSON verses and
   editors might hum like pico yet script like BASIC.

3. **Our lineage is honest:** we began with public teachings from MIT’s xv6 so
   that we could stand upon proven lessons. From that seed we have replaced,
   refit, and rewritten the majority of the orchard with our own hands.

4. **Within Dave beat many hearts:** kernel subsystems descended from our
   proprietary MTK/System OS, device hushes inspired by our embedded modem
   lines, and sundry utilities incubated in diam’s internal research initiatives. We name these borrowings that our stewardship be plain.

5. **We still honor the sages**—John Lions, the Course 6.828 scrolls, and the
   craftsmen of JOS, Plan 9, FreeBSD, and NetBSD—yet Dave’s cloak is now cut and
   sewn by diam Systems. Where code remains from MIT-PDOS we attribute; where it
   is new we rejoice.

6. **Concerning credits:** the historical roll-call of contributors to xv6
   follows below, for we would not blot out their names. Their bug reports and
   patches remain the pillars upon which our own artistry stands.

### Dave contributors (diam Systems)

- **Zohaib Ismail** *(diam Systems Ltd.)* — overall lead and architect of the
  structured-I/O push.
- **Amelia (prefers Millie. dont know why I was told to add this - just go with it.) Atkinson** *(diam Systems Ltd.)* — kernel dev. responsible for
  the scheduler refresh and multi-core bring-up.
- **Noah Campbell** *(diam Systems Ltd.)* — device and modem specialist who
  ported MTK/System drivers into Dave.
- **Ollie Cornell** *(diam Systems Ltd.)* — head of internal research,
  steward of the telemetry and tracing subsystems.
- **Izzy Sutcliffe** *(diam Systems Ltd.)* — designer of the Hello shell user
  experience and co-author of Ted.
- **Jake Bennett** — Collaborator from University of Exeter (undergrad student) who reviewed our Plan 9 and FreeBSD borrowings for clean-room compliance.
- **Joe Pitchforth** — community contributor who hardened the JSON tooling and
  built the automated test harnesses.

### Historical contributors (legacy xv6 record)

Russ Cox, Cliff Frey, Xiao Yu, Nickolai Zeldovich, Austin Clements, Silas
Boyd-Wickizer, Anton Burtsev, Cody Cutler, Mike CAT, Tej Chajed, eyalz800,
Nelson Elhage, Saar Ettinger, Alice Ferrazzi, Nathaniel Filardo, Peter
Froehlich, Yakir Goaron, Shivam Handa, Bryan Henry, Jim Huang, Alexander
Kapshuk, Anders Kaseorg, kehao95, Wolfgang Keller, Eddie Kohler, Austin Liew,
Imbar Marinescu, Yandong Mao, Matan Shabtay, Hitoshi Mitake, Carmi Merimovich,
Mark Morrissey, mtasm, Joel Nider, Greg Price, Ayan Shafqat, Eldar Sehayek,
Yongming Shen, Cam Tenny, tyfkda, Rafael Ubal, Warren Toomey, Stephen Tu, Pablo
Ventura, Xi Wang, Keiichi Watanabe, Nicolas Wolovick, wxdao, Grant Wu, Jindong
Zhang, Icenowy Zheng, Zou Chang Wei, and many unnamed friends besides.

The code in the files that constituted xv6 remains
Copyright 2006-2018 Frans Kaashoek, Robert Morris, and Russ Cox.

### Error reports

We do not process upstream xv6 bug reports (see the original notice). Feedback
about Dave may be offered directly to diam Systems Ltd.—preferably with patches
and structured reproduction steps.

BUILDING AND RUNNING DAVE

To build Dave on an x86 ELF machine (like Linux or FreeBSD), run
"make". On non-x86 or non-ELF machines (like OS X, even on x86), you
will need to install a cross-compiler gcc suite capable of producing
x86 ELF binaries (see https://pdos.csail.mit.edu/6.828/).
Then run "make TOOLPREFIX=i386-jos-elf-". Now install the QEMU PC
simulator and run "make qemu".