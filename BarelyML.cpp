/*
 ==============================================================================

 BarelyML.cpp
 Created: 5 Oct 2023
 Author:  Fritz Menzer
 Version: 0.3

 ==============================================================================
 Copyright (C) 2023-2024 Fritz Menzer

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of BarelyML and associated documentation files (the "Software"), to deal
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
 ==============================================================================
*/

#include <JuceHeader.h>
#include "BarelyML.h"

using namespace juce;

//==============================================================================

// MARK: - Display

BarelyMLDisplay::BarelyMLDisplay()
{
  // default colour palette (CGA 16 colours with some extensions)
  colours.set("black",        "#000");
  colours.set("blue",         "#00A");
  colours.set("green",        "#0A0");
  colours.set("cyan",         "#0AA");
  colours.set("red",          "#A00");
  colours.set("magenta",      "#A0A");
  colours.set("brown",        "#A50");
  colours.set("lightgray",    "#AAA");
  colours.set("darkgray",     "#555");
  colours.set("lightblue",    "#55F");
  colours.set("lightgreen",   "#5F5");
  colours.set("lightcyan",    "#5FF");
  colours.set("lightred",     "#F55");
  colours.set("lightmagenta", "#F5F");
  colours.set("yellow",       "#FF5");
  colours.set("white",        "#FFF");
  colours.set("orange",       "#FA5");
  colours.set("pink",         "#F5F");
  colours.set("darkyellow",   "#AA0");
  colours.set("purple",       "#A0F");
  colours.set("gray",         "#777");
  colours.set("linkcolour",   "#00A");
  
  // default font
  font = Font(15);
  
  // default background
  bg = Colours::white;

  // default table backgrounds
  tableBGHeader = Block::parseHexColourStatic(colours["lightcyan"], Colours::black);
  tableBG = Block::parseHexColourStatic(colours["lightgray"], Colours::black);

  // default table margins
  tableMargin = 10;
  tableGap = 2;
  
  // default list indents
  indentPerSpace = 15;
  labelGap = 30;
  
  // default content margin
  margin = 20;
  
  // default admonition margin and sizes
  iconsize = 20;
  admargin = 10;
  adlinewidth = 2;
  
  // default file source (none)
  fileSource = nullptr;

  addAndMakeVisible(viewport);
  viewport.setViewedComponent(&content, false); // we manage the content component
  viewport.setScrollBarsShown(false, false, true, false);
  viewport.setScrollOnDragMode(Viewport::ScrollOnDragMode::nonHover);
}

BarelyMLDisplay::~BarelyMLDisplay() { }

void BarelyMLDisplay::paint (Graphics& g)
{
  g.fillAll(bg);   // clear the background
}

void BarelyMLDisplay::resized()
{
  // let's keep the relative vertical position
  double relativeScrollPosition = static_cast<double>(viewport.getViewPositionY()) / content.getHeight();
  // compute content height
  int h = margin;
  for (int i=0; i<blocks.size(); i++) {
    int bh;
    bh = blocks[i]->getHeightRequired(getWidth()-2*margin)+5;  // just to be on the safe side
    if (blocks[i]->canExtendBeyondMargin()) {
      blocks[i]->setBounds(0,h,getWidth(),bh);
    } else {
      blocks[i]->setBounds(margin,h,getWidth()-2*margin,bh+10);
    }
    h += bh;
  }
  // set new bounds
  viewport.setBounds(getLocalBounds());
  content.setBounds(0,0,getWidth(), h+margin);
  // set vertical scroll position
  int newScrollY = static_cast<int>(relativeScrollPosition * content.getHeight());
  viewport.setViewPosition(0, newScrollY);
}

void BarelyMLDisplay::setMarkupString(String s) {
  markupString = s; // store for later use (e.g. if colors changed)
  blocks.clear();
  StringArray lines;
  lines.addLines(s);
  
  int li=0; // line index
  while (li<lines.size()) {
    String line=lines[li];
    if (ListItem::isListItem(line)) {               // if we find a list item...
      ListItem* b = new ListItem;                   // ...create a new object...
      b->setBMLDisplay(this);                       // ...register this display...
      b->setColours(&colours);                      // ...set its colour palette...
      if (Block::containsLink(line)) {              // ...and, if there's a link...
        line = b->consumeLink(line);                // ...preprocess line...
      }
      b->parseItemMarkup(line, font, indentPerSpace, labelGap); // ...parse it...
      content.addAndMakeVisible(b);                 // ...add the object to content component...
      blocks.add(b);                                // ...and the block list...
      li++;                                         // ...and go to next line.
    } else if (AdmonitionBlock::isAdmonitionLine(line)) {  // if we find an admonition...
      AdmonitionBlock* b = new AdmonitionBlock;     // ...create a new object...
      b->setBMLDisplay(this);                       // ...register this display...
      b->setColours(&colours);                      // ...set its colour palette...
      if (Block::containsLink(line)) {              // ...and, if there's a link...
        line = b->consumeLink(line);                // ...preprocess line...
      }
      b->parseAdmonitionMarkup(line, font, iconsize, admargin, adlinewidth); // ...parse it...
      content.addAndMakeVisible(b);                 // ...add the object to content component...
      blocks.add(b);                                // ...and the block list...
      li++;                                         // ...and go to next line.
    } else if (ImageBlock::isImageLine(line)) {     // if we find an image...
      ImageBlock* b = new ImageBlock;               // ...create a new object...
      b->setBMLDisplay(this);                       // ...register this display...
      if (Block::containsLink(line)) {              // ...and, if there's a link...
        line = b->consumeLink(line);                // ...preprocess line...
      }
      b->parseImageMarkup(line, fileSource);        // ...parse it...
      content.addAndMakeVisible(b);                 // ...add the object to content component...
      blocks.add(b);                                // ...and the block list...
      li++;                                         // ...and go to next line.
    } else if (TableBlock::isTableLine(line)) {     // if we find a table...
      TableBlock* b = new TableBlock;               // ...create a new object...
      b->setBMLDisplay(this);                       // ...register this display...
      b->setFileSource(fileSource);                 // ...give it a fileSource...
      b->setColours(&colours);                      // ...set its colour palette...
      b->setBGColours(tableBG, tableBGHeader);      // ...its background colours...
      b->setMargins(tableMargin, tableGap, margin); // ...and its margins.
      StringArray tlines;                           // set up table lines and while...
      while (TableBlock::isTableLine(line)) {       // ...current line belongs to table...
        tlines.add(line);                           // ...add it to table lines...
        line = lines[++li];                         // ...and read next line.
      }
      b->parseMarkup(tlines, font);                 // ...parse the collected lines...
      content.addAndMakeVisible(b);                 // ...add the object to content component...
      blocks.add(b);                                // ...and the block list.
    } else if (Block::containsLink(line)) {          // ...if we got here and there's a link...
      TextBlock* b = new TextBlock();               // ...set up a new text block object...
      b->setBMLDisplay(this);                       // ...register this display...
      b->setColours(&colours);                      // ...set its colours...
      line = b->consumeLink(line);                  // ...preprocess line...
      b->parseMarkup(line, font);                   // ...parse markup...
      content.addAndMakeVisible(b);                 // ...add the object to content component...
      blocks.add(b);                                // ...and the block list...
      li++;                                         // ...and go to next line.
    } else {                                        // otherwise we assume that we have a text block
      StringArray blines;                           // set up text block lines
      bool blockEnd = false;
      while (!ListItem::isListItem(line) &&         // while line is not part of a list...
             !TableBlock::isTableLine(line) &&      // ...nor a table...
             !AdmonitionBlock::isAdmonitionLine(line) && // ...nor an admonition...
             !ImageBlock::isImageLine(line) &&      // ...nor an image...
             !Block::containsLink(line) &&          // ...and doesn't contain a link...
             li<lines.size() && !blockEnd) {        // ...and we're not done yet...
        blines.add(line);                           // ...add line to text block lines...
        blockEnd = line.isEmpty();                  // ...and set up shouldEndBlock...
        line = lines[++li];                         // ...read next line...
        blockEnd &= line.isNotEmpty();              // ...and finish shouldEndBloc...
      }
      TextBlock* b = new TextBlock();               // set up a new text block object...
      b->setBMLDisplay(this);                       // ...register this display...
      b->setColours(&colours);                      // ...set its colours...
      b->parseMarkup(blines, font);                 // ...parse markup...
      content.addAndMakeVisible(b);                 // ...add the object to content component...
      blocks.add(b);                                // ...and the block list.
    }
  }
  
  resized();
}

String BarelyMLDisplay::convertFromMarkdown(String md) {
  StringArray lines;
  lines.addLines(md);
  String bml;
  bool lastLineWasTable = false;
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    // replace unspported unordered list markers
    if (line.trimStart().startsWith("* ")) {
      int idx = line.indexOf("* ");
      line = line.substring(0, idx) + "- " + line.substring(idx+2);
    }
    if (line.trimStart().startsWith("+ ")) {
      int idx = line.indexOf("+ ");
      line = line.substring(0, idx) + "- " + line.substring(idx+2);
    }
    // replace images
    while (line.contains("![") &&
        line.fromFirstOccurrenceOf("![", false, false).contains("](") &&
        line.fromLastOccurrenceOf("](", false, false).contains(")")) {
      // replace images
      int idx1 = line.indexOf("![");
      int idx2 = line.indexOf(idx1+2, "](");
      int idx3 = line.indexOf(idx2+2, ")");
      String address = line.substring(idx2+2, idx3);
      line = line.substring(0, idx1) + "{{" + address + "}}" + line.substring(idx3+2);
    }
    // replace links with labels
    while (line.contains("[") &&
               line.fromFirstOccurrenceOf("[", false, false).contains("](") &&
               line.fromLastOccurrenceOf("](", false, false).contains(")")) {
      // replace links
      int idx1 = line.indexOf("[");
      int idx2 = line.indexOf(idx1+1, "](");
      int idx3 = line.indexOf(idx2+2, ")");
      String text = line.substring(idx1+1, idx2);
      String address = line.substring(idx2+2, idx3);
      line = line.substring(0, idx1) + "[[" + address + "|" + text + "]]" + line.substring(idx3+1);
    }
    // replace links without labels
    while (line.contains("<") &&
           line.fromFirstOccurrenceOf("<", false, false).contains(">") && (
           line.fromFirstOccurrenceOf("<", false, false).startsWith("http://") ||
           line.fromFirstOccurrenceOf("<", false, false).startsWith("https://") ||
           line.fromFirstOccurrenceOf("<", false, false).startsWith("mailto:"))
          ) {
      // replace links
      int idx1 = line.indexOf("<");
      int idx2 = line.indexOf(idx1+1, ">");
      String address = line.substring(idx1+1, idx2);
      line = line.substring(0, idx1) + "[[" + address + "]]" + line.substring(idx2+1);
    }
    // when in a table, skip lines which look like this : | --- | --- |
    if (!lastLineWasTable || !(line.containsOnly("| -\t") && line.isNotEmpty())) {
      // if we found a table...
      if (line.trim().startsWith("|")) {
        // ...and this is the first line and the next line is a header separator...
        if (!lastLineWasTable && li+1<lines.size() && lines[li+1].isNotEmpty() && lines[li+1].containsOnly("| -\t") && lines[li+1].contains("-")) {
          lastLineWasTable = true;        // ...keep track of it...
          line = line.replace("|", "^");  // ...and make its cells header cells.
        }
      } else {
        lastLineWasTable = false;         // ...otherwise, keep also track.
      }
      bml += line + (li<lines.size()-1?"\n":"");
    }
  }
  // replace bold and italic markers
  String tmpBoldMarker = "%%%BarelyML%%%Bold%%%";
  bml = bml.replace("**", tmpBoldMarker);
  bml = bml.replace("__", tmpBoldMarker);
  bml = bml.replace("*", "_");            // replace italic marker
  bml = bml.replace(tmpBoldMarker, "*");  // replace temporary bold marker
  return bml;
}

String BarelyMLDisplay::convertToMarkdown(String bml) {
  StringArray lines;
  lines.addLines(bml);
  String md;
  
  bool isTable = false;
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    
    // replace table headers
    if (line.startsWith("^") && !isTable) {
      isTable = true;
      line = line.replace("^", "|");
      // count columns
      String tmp = line.substring(1);
      Array<int> colWidths;
      while (tmp.contains("|")) {
        colWidths.add(tmp.indexOf("|"));
        tmp = tmp.fromFirstOccurrenceOf("|", false, false);
      }
      if (!colWidths.isEmpty()) {
        line += "\n|";
        for (int i=0; i<colWidths.size(); i++) {
          int nhyphen = jmax(3,colWidths[i]-2);
          line += " ";
          while (nhyphen>0) {
            line += "-";
            nhyphen--;
          }
          line += " |";
        }
      }
    }
    if (line.startsWith("^") | line.startsWith("|")) {
      isTable = true;
    } else {
      isTable = false;
    }
    
    // replace links
    while (line.contains("[[") && line.fromFirstOccurrenceOf("[[", false, false).contains("]]")) {
      int idx1 = line.indexOf("[[");
      int idx2 = line.indexOf(idx1, "]]");
      String link = line.substring(idx1+2, idx2);
      if (link.contains("|")) {
        line = line.substring(0, idx1) + "[" + link.fromFirstOccurrenceOf("|", false, false) + "](" + link.upToFirstOccurrenceOf("|", false, false)  + ")" + line.substring(idx2+2);
      } else {
        line = line.substring(0, idx1) + "<" + link + ">" + line.substring(idx2+2);
      }
    }

    // add line
    md += line + (li<lines.size()-1?"\n":"");
  }
  
  // replace bold markers
  md = md.replace("*", "**");
  return md;
}

String BarelyMLDisplay::convertFromDokuWiki(String dw) {
  StringArray lines;
  lines.addLines(dw);
  String bml;
  Array<int> oLI = {1, 1, 1, 1, 1}; // ordered list indices up to 5 nesting levels supported
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    // replace headings
    bool isHeading = false;
    if (line.startsWith("====== ")) { line = "# " + line.substring(7); isHeading = true; }
    if (line.startsWith("===== ")) { line = "## " + line.substring(6); isHeading = true; }
    if (line.startsWith("==== ")) { line = "### " + line.substring(5); isHeading = true; }
    if (line.startsWith("=== ")) { line = "#### " + line.substring(4); isHeading = true; }
    if (line.startsWith("== ")) { line = "##### " + line.substring(3); isHeading = true; }
    if (isHeading) { // if we've identified a heading, let's drop the trailing markup
      while (line.endsWith(" ") || line.endsWith("=")) {
        line = line.dropLastCharacters(1);
      }
    }
    // replace ordered list markers (up to 5 nesting levels);
    int oLLevel = 0;
    bool isOL = false;
    if (line.startsWith("  - ")) { line =     String(oLI[0]) + ". " + line.substring(4); oLLevel = 1; isOL = true; }
    if (line.startsWith("    - ")) { line = " " + String(oLI[1]) + ". " + line.substring(6); oLLevel = 2; isOL = true; }
    if (line.startsWith("      - ")) { line = "  " + String(oLI[2]) + ". " + line.substring(8); oLLevel = 3; isOL = true; }
    if (line.startsWith("        - ")) { line = "   " + String(oLI[3]) + ". " + line.substring(10); oLLevel = 4; isOL = true; }
    if (line.startsWith("          - ")) { line = "    " + String(oLI[4]) + ". " + line.substring(12); oLLevel = 5; isOL = true; }
    if (isOL) { // if we've identified an ordered list item, keep track of indices
      oLI.set(oLLevel-1, oLI[oLLevel-1] + 1); // increase counter at this level
    }
    // reset the indices for deeper levels
    for (int i = oLLevel; i < 5; i++) {
      oLI.set(i, 1);
    }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("  * ")) { line = "- " + line.substring(4); }
    if (line.startsWith("    * ")) { line = " - " + line.substring(6); }
    if (line.startsWith("      * ")) { line = "  - " + line.substring(8); }
    if (line.startsWith("        * ")) { line = "   - " + line.substring(10); }
    if (line.startsWith("          * ")) { line = "    - " + line.substring(12); }
    // add line
    bml += line + (li<lines.size()-1?"\n":"");
  }
  
  // save the URLs
  bml = bml.replace("[[http://", "[[http%%%BarelyML%%%URLSEPARATOR%%%");
  bml = bml.replace("[[https://", "[[https%%%BarelyML%%%URLSEPARATOR%%%");

  // replace bold and italic markers
  bml = bml.replace("**", "*");
  bml = bml.replace("//", "_");

  // restore the URLS
  bml = bml.replace("%%%BarelyML%%%URLSEPARATOR%%%", "://");
  
  // replace color markers (supporting a subset of the "color" plugin syntax)
  bml = bml.replace("<color #", "<c#");
  bml = bml.replace("<color " , "<c:");
  bml = bml.replace("</color>", "</c>");
  
  return bml;
}

String BarelyMLDisplay::convertToDokuWiki(String bml) {
  StringArray lines;
  lines.addLines(bml);
  String dw;
  
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    // replace bold and italic markers
    line = line.replace("*", "**");
    line = line.replace("_", "//");
    // replace headings
    if (line.startsWith("# ")) { line = "====== " + line.substring(2) + " ======"; }
    if (line.startsWith("## ")) { line = "===== " + line.substring(3) + " ====="; }
    if (line.startsWith("### ")) { line = "==== " + line.substring(4) + " ===="; }
    if (line.startsWith("#### ")) { line = "=== " + line.substring(5) + " ==="; }
    if (line.startsWith("##### ")) { line = "== " + line.substring(6) + " =="; }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("- ")) { line = "  * " + line.substring(2); }
    if (line.startsWith(" - ")) { line = "    * " + line.substring(3); }
    if (line.startsWith("  - ")) { line = "      * " + line.substring(4); }
    if (line.startsWith("   - ")) { line = "        * " + line.substring(5); }
    if (line.startsWith("    - ")) { line = "          * " + line.substring(6); }
    // replace ordered list markers (up to 5 nesting levels)
    if (line.indexOf(". ")>0 && line.substring(0, line.indexOf(". ")).trim().containsOnly("0123456789")) {
      int didx = line.indexOf(". ");
      if (line.startsWith("    ") && line.substring(4, didx).containsOnly("0123456789")) { line = "          - " + line.substring(didx+2); }
      if (line.startsWith("   ") && line.substring(3, didx).containsOnly("0123456789")) { line = "        - " + line.substring(didx+2); }
      if (line.startsWith("  ") && line.substring(2, didx).containsOnly("0123456789")) { line = "      - " + line.substring(didx+2); }
      if (line.startsWith(" ") && line.substring(1, didx).containsOnly("0123456789")) { line = "    - " + line.substring(didx+2); }
      if (                       line.substring(0, didx).containsOnly("0123456789")) { line = "  - " + line.substring(didx+2); }
    }
    // add line
    dw += line + (li<lines.size()-1?"\n":"");
  }

  // replace color markers (supporting a subset of the "color" plugin syntax)
  dw = dw.replace("<c#" , "<color #");
  dw = dw.replace("<c:" , "<color ");
  dw = dw.replace("</c>", "</color>");
  
  return dw;
}

String BarelyMLDisplay::convertFromAsciiDoc(String ad) {
  StringArray lines;
  lines.addLines(ad);
  String bml;
  Array<int> oLI = {1, 1, 1, 1, 1}; // ordered list indices up to 5 nesting levels supported
  bool isTable = false;
  int tableCols = 0;
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    bool skipLine = false;
    // skip lines in square brackets (these are used for features we don't support)
    if (line.startsWith("[") && line.endsWith("]")) { skipLine = true; }
    // skip table delimiters
    if (line.startsWith("|") && line.substring(1).containsOnly("=")) { skipLine = true; isTable = !isTable; tableCols = 0; }
    // handle table
    if (!skipLine && line.startsWith("|")) {
      if (tableCols == 0) {  // first line -> contains all columns (not guaranteed for remaining lines)
        // count columns
        String tmp = line;
        while (tmp.contains("|")) {
          tmp = tmp.fromFirstOccurrenceOf("|", false, false);
          tableCols++;
        }
        // check if next line is empty
        if (li+1<lines.size() && lines[li+1].isEmpty()) { // empty -> header
          // let's remove ^ characters first, otherwise there will be alignment issues
          line = line.replace("^","").replace("|", "^") + " ^";
        } else { // not empty -> regular table row
          line = line + " |";
        }
      } else {
        // when we're here this is the first line of a non-header table row
        int colsFound = 0;
        // count columns in this line
        String tmp = line;
        while (tmp.contains("|")) {
          tmp = tmp.fromFirstOccurrenceOf("|", false, false);
          colsFound++;
        }
        // accumulate lines until we've found enough columns
        while (colsFound < tableCols && li+1<lines.size() && lines[li+1].startsWith("|") && !lines[li+1].substring(1).containsOnly("=")) {
          String nextLine = lines[++li];
          line += nextLine;
          while (nextLine.contains("|")) {
            nextLine = nextLine.fromFirstOccurrenceOf("|", false, false);
            colsFound++;
          }
        }
        line += " |";
      }
    }
    // skip empty line inside table
    if (isTable && line.isEmpty()) { skipLine = true; }
    // replace headings
    if (line.startsWith("= ")) { line = "# " + line.substring(2); }
    if (line.startsWith("== ")) { line = "## " + line.substring(3); }
    if (line.startsWith("=== ")) { line = "### " + line.substring(4); }
    if (line.startsWith("==== ")) { line = "#### " + line.substring(5); }
    if (line.startsWith("===== ")) { line = "##### " + line.substring(6); }
    // replace ordered list markers (up to 5 nesting levels);
    int oLLevel = 0;
    bool isOL = false;
    if (line.startsWith(". ")) { line =     String(oLI[0]) + ". " + line.substring(2); oLLevel = 1; isOL = true; }
    if (line.startsWith(".. ")) { line = " " + String(oLI[1]) + ". " + line.substring(3); oLLevel = 2; isOL = true; }
    if (line.startsWith("... ")) { line = "  " + String(oLI[2]) + ". " + line.substring(4); oLLevel = 3; isOL = true; }
    if (line.startsWith(".... ")) { line = "   " + String(oLI[3]) + ". " + line.substring(5); oLLevel = 4; isOL = true; }
    if (line.startsWith("..... ")) { line = "    " + String(oLI[4]) + ". " + line.substring(6); oLLevel = 5; isOL = true; }
    if (isOL) { // if we've identified an ordered list item, keep track of indices
      oLI.set(oLLevel-1, oLI[oLLevel-1] + 1); // increase counter at this level
    }
    // reset the indices for deeper levels
    for (int i = oLLevel; i < 5; i++) {
      oLI.set(i, 1);
    }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("* ")) { line = "- " + line.substring(2); }
    if (line.startsWith("** ")) { line = " - " + line.substring(3); }
    if (line.startsWith("*** ")) { line = "  - " + line.substring(4); }
    if (line.startsWith("**** ")) { line = "   - " + line.substring(5); }
    if (line.startsWith("***** ")) { line = "    - " + line.substring(6); }
    
    // replace admonitions (only NOTE and TIP, the other ones are identical)
    if (line.startsWith("NOTE: ")) { line = "INFO: " + line.substring(6); }
    if (line.startsWith("TIP: ")) { line = "HINT: " + line.substring(5); }

    // replace links
    StringArray linkSchemes = {"http://", "https://", "mailto:"};
    StringArray precedingChar = {" ", "\t"};
    for (int s=0; s<linkSchemes.size(); s++) {
      for (int p=0; p<precedingChar.size(); p++) {
        String target = precedingChar[p] + linkSchemes[s];
        while (line.contains(target) || line.startsWith(linkSchemes[s])) {
          int idx1 = line.indexOf(target)+1;
          if (idx1<0) { // if we don't find the target...
            idx1 = 0;   // ...that means the line starts with the link scheme
          }
          int idx2 = line.indexOf(idx1, " ");
          if (idx2<0) { idx2 = line.indexOf(idx1, "\t"); }
          if (idx2<0) { idx2 = line.length(); }
          // needed for cases like this: [JUCE Forum] (space in label)
          if (line.substring(idx1, idx2).contains("[")) {
            idx2 = jmax(idx2, line.indexOf(idx1, "]")+1);
          }
          String link = line.substring(idx1, idx2);
          if (link.contains("[") && link.endsWith("]")) {
            int lidx = link.indexOf("[");
            line = line.substring(0, idx1) + "[[" + link.substring(0, lidx) + "|" + link.substring(lidx+1) + "]" + line.substring(idx2);
          } else {
            line = line.substring(0, idx1) + "[[" + link + "]]" + line.substring(idx2);
          }
        }
      }
    }

    // add line
    if (!skipLine) {
      bml += line + (li<lines.size()-1?"\n":"");
    }
  }
  
  // replace bold and italic markers
  bml = bml.replace("**", "*");
  bml = bml.replace("__", "_");
  
  // replace color markers (which are actually style markers, so this is not perfectly accurate)
  while (bml.contains("]#") && bml.fromLastOccurrenceOf("]#", false, false).contains("#") && bml.upToLastOccurrenceOf("]#", false, false).contains("[")) {
    int idx2 = bml.lastIndexOf("]#");
    int idx1 = bml.substring(0, idx2).lastIndexOf("[");
    int idx3 = bml.indexOf(idx2+2, "#");
    bml = bml.substring(0, idx1) + "<c:" + bml.substring(idx1+1, idx2) + ">" + bml.substring(idx2+2,idx3) + "</c>" + bml.substring(idx3+1);
  }
  
  return bml;
}

String BarelyMLDisplay::convertToAsciiDoc(String bml) {
  StringArray lines;
  lines.addLines(bml);
  String ad;
  
  bool isTable = false;
  
  for (int li=0; li<lines.size(); li++) {
    String line = lines[li];
    // table
    if (line.startsWith("^") || line.startsWith("|")) {
      if (!isTable) { // this is the first line
        line = "|===\n" + line.replace("^", "|").upToLastOccurrenceOf("|", false, false) + "\n";
      } else {
        // drop the trailing | or ^ (note that we assume reasonable well-formedness here)
        line = line.trimEnd().dropLastCharacters(1);
      }
      isTable = true;
      if (li+1 >= lines.size() || !(lines[li+1].startsWith("|") || lines[li+1].startsWith("^"))) {
        // insert a table delimiter before the next line
        line += "\n|===";
        isTable = false;
      }
    }
    // replace headings
    if (line.startsWith("# ")) { line = "= " + line.substring(2); }
    if (line.startsWith("## ")) { line = "== " + line.substring(3); }
    if (line.startsWith("### ")) { line = "=== " + line.substring(4); }
    if (line.startsWith("#### ")) { line = "==== " + line.substring(5); }
    if (line.startsWith("##### ")) { line = "===== " + line.substring(6); }
    // replace unordered list markers (up to 5 nesting levels);
    if (line.startsWith("- ")) { line = "* " + line.substring(2); }
    if (line.startsWith(" - ")) { line = "** " + line.substring(3); }
    if (line.startsWith("  - ")) { line = "*** " + line.substring(4); }
    if (line.startsWith("   - ")) { line = "**** " + line.substring(5); }
    if (line.startsWith("    - ")) { line = "***** " + line.substring(6); }
    // replace ordered list markers (up to 5 nesting levels)
    if (line.indexOf(". ")>0 && line.substring(0, line.indexOf(". ")).trim().containsOnly("0123456789")) {
      int didx = line.indexOf(". ");
      if (line.startsWith("    ") && line.substring(4, didx).containsOnly("0123456789")) { line = "..... " + line.substring(didx+2); }
      if (line.startsWith("   ") && line.substring(3, didx).containsOnly("0123456789")) { line = ".... " + line.substring(didx+2); }
      if (line.startsWith("  ") && line.substring(2, didx).containsOnly("0123456789")) { line = "... " + line.substring(didx+2); }
      if (line.startsWith(" ") && line.substring(1, didx).containsOnly("0123456789")) { line = ".. " + line.substring(didx+2); }
      if (                       line.substring(0, didx).containsOnly("0123456789")) { line = ". " + line.substring(didx+2); }
    }
    
    // replace links
    while (line.contains("[[") && line.fromFirstOccurrenceOf("[[", false, false).contains("]]")) {
      int idx1 = line.indexOf("[[");
      int idx2 = line.indexOf(idx1, "]]");
      String link = line.substring(idx1+2, idx2);
      if (link.startsWith("http://") || link.startsWith("https://") || link.startsWith("mailto:")) {
        if (link.contains("|")) {
          line = line.substring(0, idx1) + link.upToFirstOccurrenceOf("|", false, false) + "[" + link.fromFirstOccurrenceOf("|", false, false) + "]" + line.substring(idx2+2);
        } else {
          line = line.substring(0, idx1) + link + line.substring(idx2+2);
        }
      }
    }
    
    // replace admonitions (only INFO and HINT, the other ones are identical)
    if (line.startsWith("INFO: ")) { line = "NOTE: " + line.substring(6); }
    if (line.startsWith("HINT: ")) { line = "TIP: " + line.substring(6); }

    // add line
    ad += line + (li<lines.size()-1?"\n":"");
  }
  
  // replace color markers (named colors only)
  while (ad.contains("<c:") && ad.fromFirstOccurrenceOf("<c:", false, false).contains(">") && ad.fromFirstOccurrenceOf("<c:", false, false).fromFirstOccurrenceOf(">", false, false).contains("</c>")) {
    int idx1 = ad.indexOf("<c:");
    int idx2 = ad.indexOf(idx1, ">");
    int idx3 = ad.indexOf(idx2, "</c>");
    ad = ad.substring(0, idx1) + "[" + ad.substring(idx1+3, idx2) + "]#" + ad.substring(idx2+1,idx3) + "#" + ad.substring(idx3+4);
  }
  
  return ad;
}


// MARK: - Block

Colour BarelyMLDisplay::Block::parseHexColourStatic(String s, Colour defaultColour) {
  if (s.startsWith("#")) {
    s = s.substring(1);
    // if we have 3 or 4 characters, expand by duplicating characters
    if (s.length()==3 || s.length()==4) {
      String expanded;
      for (int i=0; i<s.length(); i++) {
        expanded += s[i];
        expanded += s[i];
      }
      s = expanded;
    }
    if (s.length()==6) { // also applies to duplicated 3 char string
      s = String("FF")+s;
    }
  }
  if (s.isEmpty()) {
    return defaultColour;
  } else {
    return Colour::fromString(s);
  }
}

Colour BarelyMLDisplay::Block::parseHexColour(String s) {
  return parseHexColourStatic(s, defaultColour);
}

bool BarelyMLDisplay::Block::containsLink(String line) {
  return line.contains("[[") && line.fromFirstOccurrenceOf("[[", false, false).contains("]]");
}

String BarelyMLDisplay::Block::consumeLink(String line, String* link) {
  // output either to Block's link or "link" argument
  if (!link) { link = &this->link; }
  int idx1 = line.indexOf("[[");
  int idx2 = line.indexOf(idx1, "]]");
  if (idx1>=0 && idx2>idx1) {
    *link = line.substring(idx1+2, idx2);
    if (link->contains("|")) {
      String altText = link->fromFirstOccurrenceOf("|", false, false);
      *link = link->upToFirstOccurrenceOf("|", false, false);
      // if there's an image in the link...
      if (altText.contains("{{") && altText.fromFirstOccurrenceOf("{{", false, false).contains("}}")) {
        // ...return only the altText...
        return line.substring(0, idx1) + altText + line.substring(idx2+2);
      } else {
        // ...otherwise, return highlighted text.
        return line.substring(0, idx1) + "<c:linkcolour>*" + altText + "*</c>" + line.substring(idx2+2);
      }
    } else {
      return line.substring(0, idx1) + "<c:linkcolour>*" + *link + "*</c>" + line.substring(idx2+2);
    }
  } else {
    *link = "";
    return line;
  }
}

void BarelyMLDisplay::Block::mouseDown(const MouseEvent& event) {
  mouseDownPosition = event.position;     // keep track of position
}

void BarelyMLDisplay::Block::mouseUp(const MouseEvent& event) {
  if (link.isNotEmpty()) {                // if we have a link...
    float distance = event.position.getDistanceFrom(mouseDownPosition);
    if (distance < 20) {                  // ...and we're not scrolling...
      jassert(bmlDisplay);
      bmlDisplay->handleURL(link);        // ...let bmlDisplay handle URL.
    }
  }
}

AttributedString BarelyMLDisplay::Block::parsePureText(const StringArray& lines, Font font, bool addNewline)
{
  AttributedString attributedString;
  
  String currentLine;
  currentColour = defaultColour;
  
  bool bold = false;
  bool italic = false;
  
  for (auto line : lines)
  {
    line = line.replace("\\\\", "\n");
    if (line.startsWith("##### "))
    {
      attributedString.append(parsePureText(line.substring(6), font.boldened().withHeight(font.getHeight()*1.1f),false));
    }
    else if (line.startsWith("#### "))
    {
      attributedString.append(parsePureText(line.substring(5), font.boldened().withHeight(font.getHeight()*1.25f),false));
    }
    else if (line.startsWith("### "))
    {
      attributedString.append(parsePureText(line.substring(4), font.boldened().withHeight(font.getHeight()*1.42f),false));
    }
    else if (line.startsWith("## "))
    {
      attributedString.append(parsePureText(line.substring(3), font.boldened().withHeight(font.getHeight()*1.7f),false));
    }
    else if (line.startsWith("# "))
    {
      attributedString.append(parsePureText(line.substring(2), font.boldened().withHeight(font.getHeight()*2.1f),false));
    }
    else
    {
      while (line.isNotEmpty()) {
        bool needsNewFont = false;
        // find first token to interpret
        int bidx = line.indexOf("*");
        int iidx = line.indexOf("_");
        int tidx = line.indexOf("<");
        Colour nextColour = currentColour;
        if (bidx > -1 && (bidx < iidx | iidx == -1) && (bidx < tidx | tidx == -1)) {
          // if the next token is toggling the bold state...
          // ...first add everything up to the token...
          attributedString.append(line.substring(0, bidx), font, currentColour);
          line = line.substring(bidx+1); // ...then drop up to and including the token...
          bold = !bold;                  // ...toggle the bold status...
          needsNewFont = true;           // ...and request new font.
        } else if (iidx > -1 && (iidx < tidx | tidx == -1)) {
          // if the next token is toggling the italic state...
          // ...first add everything up to the token...
          attributedString.append(line.substring(0, iidx), font, currentColour);
          line = line.substring(iidx+1); // ...then drop up to and including the token...
          italic = !italic;              // ...toggle the italic status...
          needsNewFont = true;           // ...and request new font.
        } else if (tidx > -1) {
          // if the next token is a tag, first figure out if it is a recognized tag...
          String tag;
          bool tagRecognized = false;
          // find tag end
          int tidx2 = line.indexOf(tidx, ">");
          if (tidx2>tidx) {
            tag = line.substring(tidx+1, tidx2);
          }
          if (tag.startsWith("c#")) {
            // hex colour tag
            nextColour = parseHexColour(tag.substring(1));
            tagRecognized = true;
          } else if (tag.startsWith("c:")) {
            // named colour tag
            String name = tag.substring(2);
            if (colours != nullptr && colours->containsKey(name)) {
              nextColour = parseHexColour((*colours)[name]);
            }
            tagRecognized = true;
          } else if (tag.startsWith("/c")) {
            // end of colour tag
            nextColour = defaultColour;
            tagRecognized = true;
          }
          if (tagRecognized) {
            // ...first add everything up to the tag...
            attributedString.append(line.substring(0, tidx), font, currentColour);
            // ...then drop up to and including the tag.
            line = line.substring(tidx2+1);
          } else {
            // ...first add everything up to and including the token...
            attributedString.append(line.substring(0, tidx+1), font, currentColour);
            // ...then drop it.
            line = line.substring(tidx+1);
          }
        } else {
          // if no token was found -> add the remaining text...
          attributedString.append(line, font, currentColour);
          // ...and clear the line.
          line.clear();
        }
        currentColour = nextColour;
        if (needsNewFont) {
          font = font.withStyle(Font::plain);
          if (bold) { font = font.boldened(); }
          if (italic) { font = font.italicised(); }
        }
      }
    }
    
    if (addNewline) {
      attributedString.append(" \n", font, defaultColour);
    }
  }
  return attributedString;
}

// MARK: - Text Block

void BarelyMLDisplay::TextBlock::parseMarkup(const StringArray& lines, Font font) {
  attributedString = parsePureText(lines, font);
}

float BarelyMLDisplay::TextBlock::getHeightRequired(float width) {
  TextLayout layout;
  layout.createLayout(attributedString, width);
  return layout.getHeight();
}

void BarelyMLDisplay::TextBlock::paint(juce::Graphics& g) {
  attributedString.draw(g, getLocalBounds().toFloat());
}

// MARK: - Admonition Block

bool BarelyMLDisplay::AdmonitionBlock::isAdmonitionLine(const String& line) {
  return line.startsWith("INFO: ") || line.startsWith("HINT: ") || line.startsWith("IMPORTANT: ") || line.startsWith("CAUTION: ") || line.startsWith("WARNING: ");
}

void BarelyMLDisplay::AdmonitionBlock::parseAdmonitionMarkup(const String& line, Font font, int iconsize, int margin, int linewidth) {
  if (line.startsWith("INFO: ")) { type = info; }
  else if (line.startsWith("HINT: ")) { type = hint; }
  else if (line.startsWith("IMPORTANT: ")) { type = important; }
  else if (line.startsWith("CAUTION: ")) { type = caution; }
  else if (line.startsWith("WARNING: ")) { type = warning; }
  attributedString = parsePureText(line.fromFirstOccurrenceOf(": ", false, false), font);
  this->iconsize = iconsize;
  this->margin = margin;
  this->linewidth = linewidth;
}

float BarelyMLDisplay::AdmonitionBlock::getHeightRequired(float width) {
  TextLayout layout;
  layout.createLayout(attributedString, width-iconsize-2*(margin+linewidth));
  return jmax(layout.getHeight(),(float)iconsize);
}

void BarelyMLDisplay::AdmonitionBlock::paint(juce::Graphics& g) {
  // select colour
  switch (type) {
    case info:
      g.setColour(parseHexColour((*colours)["blue"]));
      break;
      
    case hint:
      g.setColour(parseHexColour((*colours)["green"]));
      break;
      
    case important:
      g.setColour(parseHexColour((*colours)["red"]));
      break;
      
    case caution:
      g.setColour(parseHexColour((*colours)["yellow"]));
      break;
      
    case warning:
      g.setColour(parseHexColour((*colours)["orange"]));
      break;
  }
  // draw tab
  g.fillRect(Rectangle<int>(0,0,iconsize,iconsize));
  // draw lines left and right
  g.fillRect(Rectangle<int>(iconsize,0,linewidth,getHeight()));
  g.fillRect(Rectangle<int>(getWidth()-linewidth,0,linewidth,getHeight()));
  attributedString.draw(g, Rectangle<float>(iconsize+margin+linewidth,
                                          0,
                                          getWidth()-iconsize-2*(margin+linewidth),
                                          getHeight()));
}


// MARK: - Table Block

BarelyMLDisplay::TableBlock::TableBlock() {
  addAndMakeVisible(viewport);
  viewport.setViewedComponent(&table, false); // we manage the content component
  viewport.setScrollBarsShown(false, false, false, true); // scroll only horizontally
  viewport.setScrollOnDragMode(Viewport::ScrollOnDragMode::nonHover);
}

bool BarelyMLDisplay::TableBlock::isTableLine(const String& line) {
  return line.startsWith("^") || line.startsWith("|");
}

void BarelyMLDisplay::TableBlock::parseMarkup(const StringArray& lines, Font font) {
  // read cells
  table.cells.clear();
  table.setBMLDisplay(bmlDisplay);
  for (auto line : lines) {
    // find all cells in this line
    OwnedArray<Cell>* row = new OwnedArray<Cell>();
    while (line.containsAnyOf("^|")) {
      bool isHeader = line.startsWith("^");
      line = line.substring(1);                     // remove left delimiter
      int nextDelimiter = line.indexOfAnyOf("^|");  // find right delimiter
      if (nextDelimiter>=0) {                       // no delimiter found -> we're done with this line
        String rawString = line.substring(0, nextDelimiter);
        // fix delimiter if we have broken apart a link
        if (rawString.contains("[[") && !rawString.contains("]]") && line.contains("]]")) {
          int linkEnd = line.indexOf("]]");
          int idx1 = line.indexOf(linkEnd, "|");
          int idx2 = line.indexOf(linkEnd, "^");
          nextDelimiter = linkEnd;
          if (idx1>=0 && (idx2<0 || idx1<idx2)) { nextDelimiter = idx1; }
          if (idx2>=0 && (idx1<0 || idx2<idx1)) { nextDelimiter = idx2; }
          rawString = line.substring(0, nextDelimiter);
        }
        line = line.substring(nextDelimiter);         // drop everything up to right delimiter
        // TODO: use the number of whitespace characters on either side of rawString to determine justification
        // TODO: implement || -> previous cell spans two columns
        // check for link
        String trimmed = rawString.trim();
        String cellLink;
        if (trimmed.contains("[[") && trimmed.fromFirstOccurrenceOf("[[", false, false).contains("]]")) {
          trimmed = consumeLink(trimmed, &cellLink);
        }
        // check for image
        int width = -1;
        std::unique_ptr<Drawable> drawable;
        if (trimmed.startsWith("{{") && trimmed.endsWith("}}")) {
          String filename = trimmed.fromFirstOccurrenceOf("{{", false, false).upToFirstOccurrenceOf("}}", false, false);
          if (filename.contains("?")) {
            width = filename.fromFirstOccurrenceOf("?", false, false).getIntValue();
            filename = filename.upToFirstOccurrenceOf("?", false, false);
          }
          if (fileSource) {
            drawable = fileSource->getDrawableForFilename(filename);
            if (!drawable) {
              trimmed += String(" File not found.");
            }
          } else {
            trimmed += String(" No file source.");
          }
        }
        AttributedString attributedString = parsePureText(trimmed, isHeader?font.boldened():font);
        TextLayout layout;
        layout.createLayout(attributedString, 1.0e7f);
        if (width>0 && drawable && drawable->getDrawableBounds().getWidth()>0.f) {
          float w = drawable->getDrawableBounds().getWidth();
          float h = drawable->getDrawableBounds().getHeight();
          row->add(new Cell {attributedString, std::move(drawable), cellLink, isHeader, (float)width, width*h/w});
        } else {
          row->add(new Cell {attributedString, std::move(drawable), cellLink, isHeader, layout.getWidth(), layout.getHeight()});
        }
      }
    }
    table.cells.add(row);
  }
  // compute column widths
  table.columnwidths.clear();
  for (int i=0; i<table.cells.size(); i++) {
    OwnedArray<Cell>* row = table.cells[i];
    for (int j=0; j<row->size(); j++) {
      if (j<table.columnwidths.size()) {
        table.columnwidths.set(j, jmax(table.columnwidths[j],(*row)[j]->width));
      } else {
        table.columnwidths.set(j, (*row)[j]->width);
      }
    }
  }
  // compute row heights
  table.rowheights.clear();
  for (int i=0; i<table.cells.size(); i++) {
    OwnedArray<Cell>* row = table.cells[i];
    float rowheight = 0;
    for (int j=0; j<row->size(); j++) {
      rowheight = jmax(rowheight,(*row)[j]->height);
    }
    table.rowheights.set(i, rowheight);
  }
  table.setBounds(0, 0, getWidthRequired()+table.leftmargin+table.cellgap, getHeightRequired(0.f));
}

float BarelyMLDisplay::TableBlock::getWidthRequired() {
  float width = 0;
  for (int i=0; i<table.columnwidths.size(); i++) {
    width += table.columnwidths[i] + 2 * table.cellmargin + table.cellgap;
  }
  return width - table.cellgap;
}

float BarelyMLDisplay::TableBlock::getHeightRequired(float width) {
  // NOTE: We're ignoring width - the idea is that tables can be scrolled horizontally if necessary
  float height = 0;
  for (int i=0; i<table.rowheights.size(); i++) {
    height += table.rowheights[i] + 2 * table.cellmargin + table.cellgap;
  }
  return height-table.cellgap;
}

void BarelyMLDisplay::TableBlock::resized() {
  viewport.setBounds(getLocalBounds());
}

void BarelyMLDisplay::TableBlock::Table::paint(juce::Graphics& g) {
  float y = 0.f;    // Y coordinate of cell's top left corner
  for (int i=0; i<cells.size(); i++) {
    float x = leftmargin;  // X coordinate of cell's top left corner
    OwnedArray<Cell>* row = cells[i]; // get current row
    for (int j=0; j<row->size(); j++) {
      Cell* c = (*row)[j];            // get current cell
      if (c->isHeader) {               // if it's a header cell...
        g.setColour(bgHeader);        // ...set header background colour
      } else {                        // otherwise...
        g.setColour(bg);              // ...set regular background colour
      }
      // fill background
      g.fillRect(x, y, columnwidths[j] + 2 * cellmargin, rowheights[i] + 2 * cellmargin);
      Rectangle<float> destArea = Rectangle<float>(x+cellmargin, y+cellmargin, columnwidths[j], rowheights[i]);
      if (c->drawable) {
        // draw drawable
        c->drawable->drawWithin(g, destArea, RectanglePlacement::centred, 1.0f);
      } else {
        // draw cell text
        c->s.draw(g, destArea);
      }
      // move one cell to the right
      x += columnwidths[j] + 2 * cellmargin + cellgap;
    }
    // move to next row (note: x will be reset at next loop iteration)
    y += rowheights[i] + 2 * cellmargin + cellgap;
  }
}

void BarelyMLDisplay::TableBlock::Table::mouseDown(const MouseEvent& event) {
  mouseDownPosition = event.position;     // keep track of position
}

void BarelyMLDisplay::TableBlock::Table::mouseUp(const MouseEvent& event) {
  String link;
  // find link for mouseDownPosition
  float mdy = mouseDownPosition.y;
  float mdx = mouseDownPosition.x;
  float y = 0.f;                    // Y coordinate of row's top edge
  for (int i=0; i<cells.size(); i++) {
    if (mdy>=y && mdy<y+rowheights[i] + 2 * cellmargin) {
      float x = leftmargin;         // X coordinate of cell's left edge
      for (int j=0; j<cells[i]->size(); j++) {
        if (mdx>=x && mdx<x+columnwidths[j] + 2 * cellmargin) {
          link = (*cells[i])[j]->link;
        }
        // move one cell to the right
        x += columnwidths[j] + 2 * cellmargin + cellgap;
      }
    }
    // move to next row
    y += rowheights[i] + 2 * cellmargin + cellgap;
  }
  if (link.isNotEmpty()) {          // if we have a link...
    float distance = event.position.getDistanceFrom(mouseDownPosition);
    if (distance < 20) {            // ...and we're not scrolling...
      jassert(bmlDisplay);
      bmlDisplay->handleURL(link);  // ...let bmlDisplay handle URL.
    }
  }
}



// MARK: - Image Block
bool BarelyMLDisplay::ImageBlock::isImageLine(const String& line) {
  return (line.startsWith("{{") && line.trim().endsWith("}}")) || // either just an image...
         (line.startsWith("[[") && line.trim().endsWith("]]") &&  // ...or a link around...
          line.contains("{{") && line.fromFirstOccurrenceOf("{{", false, false).contains("}}")); // ...an image.
}

void BarelyMLDisplay::ImageBlock::parseImageMarkup(const String& line, FileSource* fileSource) {
  String filename = line.fromFirstOccurrenceOf("{{", false, false).upToFirstOccurrenceOf("}}", false, false);
  if (filename.contains("?")) {
    maxWidth = filename.fromFirstOccurrenceOf("?", false, false).getIntValue();
    filename = filename.upToFirstOccurrenceOf("?", false, false);
  } else {
    maxWidth = -1;
  }
  if (fileSource) {
    drawable = fileSource->getDrawableForFilename(filename);
  } else {
    imageMissingMessage.append("no file source. ", Font(14), defaultColour);
  }
  if (!drawable) {
    imageMissingMessage.append(filename + " not found.", Font(14), defaultColour);
  }
}

float BarelyMLDisplay::ImageBlock::getHeightRequired(float width) {
  if (drawable && drawable->getDrawableBounds().getWidth()>0.f) {
    float w = drawable->getDrawableBounds().getWidth();
    float h = drawable->getDrawableBounds().getHeight();
    if (maxWidth>0) {
      return jmin((float)maxWidth,width)*h/w;
    } else {
      return width*h/w;
    }
  } else {
    return 20.f;
  }
}

void BarelyMLDisplay::ImageBlock::paint(juce::Graphics& g) {
  if (drawable) {
    float w = getWidth();
    if (maxWidth>0) {
      w = jmin((float)maxWidth,w);
    }
    drawable->drawWithin(g, Rectangle<float>(0, 0, w, getHeight()), RectanglePlacement::centred, 1.0f);
  } else {
    g.setColour(defaultColour);
    g.drawRect(getLocalBounds());
    g.drawLine(0, 0, getWidth(), getHeight());
    g.drawLine(getWidth(), 0, 0, getHeight());
    imageMissingMessage.draw(g, getLocalBounds().reduced(5,5).toFloat());
  }
}

void BarelyMLDisplay::ImageBlock::resized() {
}


// MARK: - List Item

 bool BarelyMLDisplay::ListItem::isListItem(const String& line) {
  return (line.indexOf(". ")>0 && line.substring(0, line.indexOf(". ")).trim().containsOnly("0123456789")) || (line.indexOf("- ")>=0 && !line.substring(0, line.indexOf("- ")).containsNonWhitespaceChars());
}

void BarelyMLDisplay::ListItem::parseItemMarkup(const String& line, Font font, int indentPerSpace, int gap) {
  this->gap  = gap;
  label.clear();
  
  int dotidx = line.indexOf(". ");                      // find dot+space in line
  String beforedot = line.substring(0, dotidx);         // find out if before the dot...
  String lbl = beforedot.trimStart();                   // ...there's only whitespace...
  if (dotidx>0 && lbl.containsOnly("0123456789")) {     // ...and at least one number.
    label.append(lbl+".", font, defaultColour);          // create label
    // parse item text (everything after the dot)
    attributedString = parsePureText(line.substring(dotidx+2).trimStart(), font);
    // use number of whitespace characters to determine indent
    indent = indentPerSpace * (beforedot.length()-lbl.length());
  } else {                                              // otherwise try unordered list:
    int hyphenidx = line.indexOf("- ");                 // find hyphen+space in line
    String beforehyphen = line.substring(0, hyphenidx); // find out if before the hyphen...
    if (!beforehyphen.containsNonWhitespaceChars()) {   // ...there's only whitespace.
      // parse item text (everything after the hyphen)
      attributedString = parsePureText(line.substring(hyphenidx+2).trimStart(), font);
      // use number of whitespace characters to determine indent
      indent = indentPerSpace * beforehyphen.length();
      // create label TODO: have bullet character depend on indent
      label.append(CharPointer_UTF8("•"), font, defaultColour);
    } else {  // if everything fails, interpret as regular text without label
      indent = 0;
      attributedString = parsePureText(line, font);
    }
  }
}

float BarelyMLDisplay::ListItem::getHeightRequired(float width) {
  TextLayout layout;
  layout.createLayout(attributedString, width-indent-gap);
  return layout.getHeight();
}

void BarelyMLDisplay::ListItem::paint(juce::Graphics& g) {
//  g.fillAll(Colours::lightgreen);   // clear the background
  label.draw(g, getLocalBounds().withTrimmedLeft(indent).toFloat());
  attributedString.draw(g, getLocalBounds().withTrimmedLeft(indent+gap).toFloat());
}
