Thanks for contributing to International Doom! Whatever your contribution,
whether it's code or just a bug report, it's greatly appreciated.

The project is governed by the
[Contributor Covenant](http://contributor-covenant.org/version/1/4/)
version 1.4. By contributing to the project you agree to abide by its
terms. To report violations, please send an email to julia.nechaevskaya@live.com.

### Reporting bugs

Try searching [the GitHub issues
list](https://github.com/JNechaevsky/inter-doom/issues) to see
if your bug has already been reported.

If you're confident that you've found a bug (or even if you're
not sure!) please go ahead and [file an issue on
GitHub](https://github.com/JNechaevsky/inter-doom/issues/new/choose).
You'll need a GitHub account, but it's pretty easy to sign up.

Please try to give as much information as possible:

* What version of International Doom are you using? You can check version
  by running International Doom with --version command line argument or by using VERSION ingame cheat.

* International Doom runs on many different operating systems (not just
  Windows!). Please say which operating system and what version of it
  you're using.

* Please say which game you're playing (Doom 1, Doom 2, Heretic,
  Hexen, Strife, etc.) and if you're using any fan-made WADs or mods,
  please say which mods (and where they can be downloaded!). It helps
  to give the full command line you're using to start the game.

* Please mention if you have any special configuration you think may be
  relevant, too.

### Feature requests

International Doom is always open to new feature requests.
To make a feature request, [file an issue on
GitHub](https://github.com/JNechaevsky/inter-doom/issues/new/choose).

### Bug fixes / code submission

Thank you for contributing code to International Doom! Please check the
following guidelines before opening a pull request:

* All code must be licensed under [the GNU General Public License,
  version 2](https://www.gnu.org/licenses/old-licenses/gpl-2.0.html).
  Please don't reuse code that isn't GPL, or that is GPLv3 licensed.
  Be aware that by submitting your code to the project, you're agreeing
  to license it under the GPL.

* Please don't make unnecessary changes which just change formatting
  without any actual change to program logic. While being consistent
  is nice, such changes destroy the ability to use the `git blame`
  command to see when code was last changed.

* If you're not sure that your feature will be accepted, open a
  feature request first and ask before you start implementing your
  feature.

* Follow the guidelines for [how to write a Git commit
  message](http://chris.beams.io/posts/git-commit/). In short: the
  first line should be a short summary;
  use the imperative mood ("fix bug X", rather than "fixed bug X" or
  "fixing bug X"). If your change fixes a particular subsystem,
  prefix the summary with that subsystem: eg. "Doom: Fix bug X" or
  "Textscreen: Change size of X".

* If you're making a change related to a bug, reference the GitHub
  issue number in the commit message, e.g., "This is a partial fix
  for #646". This will link your commit into the issue comments. If
  your change is a fix for the bug, put the word "fixes" before the
  issue number to automatically close the issue once your change is merged.  
  **NOTE:** If you frequently use `git rebase` it is better to reference
  the GitHub issue number in the Pull Request description rather than
  in the commit message to avoid cluttering the issues with links
  to outdated commits.
