/*
 ==============================================================================
 
 BarelyML.h
 Created: 5 Oct 2023
 Author:  Fritz Menzer
 Version: 0.1
 
 ==============================================================================
 Copyright (C) 2023 Fritz Menzer

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

 BarelyML.h and BarelyML.cpp implement the BarelyML markup language, which
 supports the following syntax:
 
 Headings

 # Level 1 Heading
 ## Level 2 Heading
 ### Level 3 Heading
 #### Level 4 Heading
 ##### Level 5 Heading


 Bold and Italic

 *Bold Text*
 _Italic Text_


 Unordered lists with hyphens

 - Item 1
 - Item 2


 Ordered lists with numbers

 1. Item 1
 2. Item 2


 Tables

 ^ Header 1      ^ Header 2     ^
 | Cell 1-1      | Cell 1-2     |
 ^ Also a header | Not a header |


 Font Colour

 <c:red>Red Text</c>
 <c#FFFFFF>White Text</c>

 Colour names supported by default (CGA 16-colour palette with some extensions):
 black,blue,green,cyan,red,magenta,brown,lightgray,
 darkgray,lightblue,lightgreen,lightcyan,lightred,lightmagenta,yellow,white,
 orange, pink, darkyellow, purple, gray, linkcolour (by default set to blue)
 (the idea is that there will be the option to provide a custom colour definition object)

 
 Images

 {{image-filename.jpg?200}}
 
 The number after the "?" is the maximum width (optional).
 
 
 Links
 
 [[https://mnsp.ch|My Website]]
 
 
 Admonitions

 INFO: This is an info paragraph (blue tab).
 HINT: This is a hint paragraph (green tab).
 IMPORTANT: This is an important paragraph (red tab).
 CAUTION: This is a caution paragraph (yellow tab).
 WARNING: This is a warning paragraph (orange tab).

 TODO: Links in tables
 TODO: Images in tables
 TODO: Add support for more markup formats
 TODO: Icons for admonitions

 ==============================================================================
 */

#pragma once

using namespace juce;

#include <JuceHeader.h>

//==============================================================================
class BarelyMLDisplay  : public Component
{
public:
  BarelyMLDisplay();
  ~BarelyMLDisplay() override;
  
  void paint (juce::Graphics&) override;
  void resized() override;
  
  void setMarkupString(String s, Font font = Font(15.0f));
  void setMargin(int m) { margin = m; };
  void setColours(StringPairArray c) { colours = c; };
  void setBGColour(Colour bg) { this->bg = bg; };
  void setTableColours(Colour bg, Colour bgHeader) { tableBG = bg; tableBGHeader = bgHeader; };
  void setTableMargins(int margin, int gap) { tableMargin = margin; tableGap = gap; };
  void setListIndents(int indentPerSpace, int labelGap) {
    this->indentPerSpace = indentPerSpace;
    this->labelGap = labelGap;
  };
  void setAdmonitionSizes(int iconsize, int admargin, int adlinewidth) {
    this->iconsize = iconsize;
    this->admargin = admargin;
    this->adlinewidth = adlinewidth;
  };

  static String convertFromMarkdown(String md);
  void setMarkdownString(String md, Font font = Font(15.0f)) {
    setMarkupString(convertFromMarkdown(md), font);
  }

  /*static String convertFromDokuWiki(String dw);
  void setDokuWikiString(String dw, Font font = Font(15.0f)) {
    setMarkupString(convertFromDokuWiki(dw), font);
  }

  static String convertFromAsciiDoc(String ad);
  void setAsciiDocString(String ad, Font font = Font(15.0f)) {
    setMarkupString(convertFromDokuWiki(ad), font);
  }*/

  class FileSource {
  public:
    virtual ~FileSource() {};
    virtual Image getImageForFilename(String filename) = 0;
  };
  
  void setFileSource(FileSource* fs) { fileSource = fs; }
  
private:
  class Block : public Component
  {
  public:
    Block ()  { colours = nullptr; defaultColour = Colours::black; }
    // static utility methods
    static Colour parseHexColourStatic(String s, Colour defaultColour);
    static bool containsLink(String line);
    // Common functionalities for all blocks
    String consumeLink(String line);
    virtual void parseMarkup(const StringArray& lines, Font font) {};
    virtual float getHeightRequired(float width) = 0;
    void setColours(StringPairArray* c) { colours = c; };
    virtual bool canExtendBeyondMargin() { return false; }; // for tables
    // mouse handlers for clicking on links
    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;

  protected:
    AttributedString parsePureText(const StringArray& lines, Font font);
    Colour defaultColour;
    Colour currentColour;
    StringPairArray* colours;
    Colour parseHexColour(String s);

  private:
    String link;
    Point<float> mouseDownPosition;
  };
  
  class TextBlock  : public Block
  {
  public:
    void parseMarkup(const StringArray& lines, Font font) override;
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
  private:
    AttributedString attributedString;
  };
  
  class AdmonitionBlock  : public Block
  {
  public:
    static bool isAdmonitionLine(const String& line);
    void parseAdmonitionMarkup(const String& line, Font font, int iconsize, int margin, int linewidth);
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
  private:
    AttributedString attributedString;
    enum ParagraphType { info, hint, important, caution, warning };
    ParagraphType type;
    int iconsize, margin, linewidth;
  };
  
  class TableBlock : public Block
  {
  public:
    TableBlock ();
    static bool isTableLine(const String& line);
    void parseMarkup(const StringArray& lines, Font font) override;
    float getWidthRequired();
    float getHeightRequired(float width) override;
    void resized() override;
    void setBGColours(Colour bg, Colour bgHeader) {
      table.bg = bg;
      table.bgHeader = bgHeader;
    }
    void setMargins(int margin, int gap, int leftmargin) {
      table.cellmargin = margin;
      table.cellgap = gap;
      table.leftmargin = leftmargin;
    }
    bool canExtendBeyondMargin() override { return true; };
  private:
    typedef struct {
      AttributedString s;
      bool isHeader;
      float width;
      float height;
    } Cell;
    class InnerViewport : public Viewport {
    public:
      // Override the mouse event methods to forward them to the parent Viewport
      void mouseDown(const MouseEvent& e) override {
        if (Viewport* parent = findParentComponentOfClass<Viewport>()) {
          MouseEvent ep = MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseDown(ep);
        }
        Viewport::mouseDown(e);
      }
      void mouseUp(const MouseEvent& e) override {
        if (Viewport* parent = findParentComponentOfClass<Viewport>()) {
          MouseEvent ep = MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseUp(ep);
        }
        Viewport::mouseUp(e);
      }
      void mouseDrag(const MouseEvent& e) override {
        if (Viewport* parent = findParentComponentOfClass<Viewport>()) {
          MouseEvent ep = MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseDrag(ep);
        }
        Viewport::mouseDrag(e);
      }
      // Override mouseWheelMove to forward events to the parent Viewport
      void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& wheel) override {
        Viewport* parent = findParentComponentOfClass<Viewport>();
        if (parent != nullptr) {
          MouseEvent ep = MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseWheelMove(ep, wheel);
        }
        Viewport::mouseWheelMove(e, wheel);
      }
    };
    class Table : public Component {
    public:
      void paint(juce::Graphics&) override;
      OwnedArray<OwnedArray<Cell>> cells;
      Array<float> columnwidths;
      Array<float> rowheights;
      Colour bg, bgHeader;
      int cellmargin, cellgap, leftmargin;
    };
    InnerViewport viewport;
    Table table;
  };
  
  class ImageBlock : public Block
  {
  public:
    static bool isImageLine(const String& line);
    void parseImageMarkup(const String& line, FileSource* fileSource);
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
    void resized() override;
  private:
    AttributedString imageMissingMessage;
    Image image;
    int maxWidth;
  };
  
  class ListItem : public Block
  {
  public:
    static bool isListItem(const String& line);
    void parseItemMarkup(const String& line, Font font, int indentPerSpace, int gap);
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
  private:
    AttributedString attributedString;
    AttributedString label;
    int indent;
    int gap;
  };
  
  StringPairArray colours;        // colour palette
  Colour bg;                      // background colour
  Colour tableBG, tableBGHeader;  // table background colours
  int tableMargin, tableGap;      // table margins
  int indentPerSpace, labelGap;   // list item indents
  Viewport  viewport;             // a viewport to scroll the content
  Component content;              // a component with the content
  OwnedArray<Block> blocks;       // representation of the document as blocks
  int margin;                     // content margin in pixels
  int iconsize;                   // admonition icon size in pixels
  int admargin;                   // admonition margin in pixels
  int adlinewidth;                // admonition line width in pixels
  FileSource* fileSource;         // data source for image files, etc.
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarelyMLDisplay)
};
