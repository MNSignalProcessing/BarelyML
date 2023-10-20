# BarelyML

Markup display for JUCE - Barely a Markup Language

Introducing BarelyML, the markup language nobody asked for but you're getting anyway! When I realized that just implementing a Markdown display for JUCE apps would not give me the flexibility I needed, I combined my favourite features from different markup languages (e.g. the headers from Markdown and the tables from DokuWiki) and added a simple coloring scheme on top.

As a plus, BarelyMLDisplay can convert simple Markdown, DokuWiki, and AsciiDoc content to its own syntax on the fly, and comes with a handy interactive demo project (as a PIP => just drag BarelyMLDemo.h onto a Projucer window).

## Version history

### 0.2 (2023-10-20)
- Comes now with an interactice demo (as a PIP for the Projucer)
- Fixes a bug where text may be not displayed (see [JUCE Forum](https://forum.juce.com/t/attributedstring-last-line-disappearing-when-preceding-line-was-wrapped/58396))
- Removed juce namespace from BarelyML.h
- Added conversion methods for DokuWiki and AsciiDoc

### 0.1 (2023-10-08)
- initial release

Licensed under the MIT license, in the hope it will be useful to as many people as possible.
