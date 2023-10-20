/*
 ==============================================================================
 
 BarelyML.h
 Created: 5 Oct 2023
 Author:  Fritz Menzer
 Version: 0.2.1
 
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
 TODO: Icons for admonitions
 
 NOTE: The conversion methods FROM OTHER FORMATS TO BarelyML are incomplete,
       but work for most simple documents. If you have a use case that doesn't
       work yet, please let me know via GitHub or the JUCE forum and I'll try
       to make it work.
  
       The conversion methods FROM BarelyML TO OTHER FORMATS on the other hand
       are extremely minimal and only used in the demo application to keep the
       UI from doing weird stuff when switching the markdown language. For now
       I don't see any other use, so don't count on this becoming a feature.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>

//==============================================================================
class BarelyMLDisplay  : public juce::Component
{
public:
  BarelyMLDisplay();
  ~BarelyMLDisplay() override;
  
  // MARK: - juce::Component Methods
  void paint (juce::Graphics&) override;
  void resized() override;
  
  // MARK: - Parameters
  void setFont(juce::Font font) { this->font = font; };
  void setMargin(int m) { margin = m; };
  void setColours(juce::StringPairArray c) { colours = c; };
  void setBGColour(juce::Colour bg) { this->bg = bg; };
  void setTableColours(juce::Colour bg, juce::Colour bgHeader) { tableBG = bg; tableBGHeader = bgHeader; };
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

  // MARK: - Format Conversion (static methods)
  static juce::String convertFromMarkdown(juce::String md);
  static juce::String convertToMarkdown(juce::String bml);

  static juce::String convertFromDokuWiki(juce::String dw);
  static juce::String convertToDokuWiki(juce::String bml);
  
  static juce::String convertFromAsciiDoc(juce::String ad);
  static juce::String convertToAsciiDoc(juce::String bml);

  // MARK: - Content
  void setMarkupString(juce::String s);
  void setMarkdownString(juce::String md) { setMarkupString(convertFromMarkdown(md)); }
  void setDokuWikiString(juce::String dw) { setMarkupString(convertFromDokuWiki(dw)); }
  void setAsciiDocString(juce::String ad) { setMarkupString(convertFromDokuWiki(ad)); }


  // MARK: - File Handling (for images)
  class FileSource {
  public:
    virtual ~FileSource() {};
    virtual juce::Image getImageForFilename(juce::String filename) = 0;
  };
  
  void setFileSource(FileSource* fs) { fileSource = fs; }
  
private:
  // MARK: - Blocks
  class Block : public Component
  {
  public:
    Block ()  { colours = nullptr; defaultColour = juce::Colours::black; }
    // static utility methods
    static juce::Colour parseHexColourStatic(juce::String s, juce::Colour defaultColour);
    static bool containsLink(juce::String line);
    // Common functionalities for all blocks
    juce::String consumeLink(juce::String line);
    virtual void parseMarkup(const juce::StringArray& lines, juce::Font font) {};
    virtual float getHeightRequired(float width) = 0;
    void setColours(juce::StringPairArray* c) { 
      colours = c;
      defaultColour = parseHexColour((*colours)["default"]);
    };
    virtual bool canExtendBeyondMargin() { return false; }; // for tables
    // mouse handlers for clicking on links
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

  protected:
    juce::AttributedString parsePureText(const juce::StringArray& lines, juce::Font font, bool addNewline = true);
    juce::Colour defaultColour;
    juce::Colour currentColour;
    juce::StringPairArray* colours;
    juce::Colour parseHexColour(juce::String s);

  private:
    juce::String link;
    juce::Point<float> mouseDownPosition;
  };
  
  class TextBlock  : public Block
  {
  public:
    void parseMarkup(const juce::StringArray& lines, juce::Font font) override;
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
  private:
    juce::AttributedString attributedString;
  };
  
  class AdmonitionBlock  : public Block
  {
  public:
    static bool isAdmonitionLine(const juce::String& line);
    void parseAdmonitionMarkup(const juce::String& line, juce::Font font, int iconsize, int margin, int linewidth);
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
  private:
    juce::AttributedString attributedString;
    enum ParagraphType { info, hint, important, caution, warning };
    ParagraphType type;
    int iconsize, margin, linewidth;
  };
  
  class TableBlock : public Block
  {
  public:
    TableBlock ();
    static bool isTableLine(const juce::String& line);
    void parseMarkup(const juce::StringArray& lines, juce::Font font) override;
    float getWidthRequired();
    float getHeightRequired(float width) override;
    void resized() override;
    void setBGColours(juce::Colour bg, juce::Colour bgHeader) {
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
      juce::AttributedString s;
      bool  isHeader;
      float width;
      float height;
    } Cell;
    class InnerViewport : public juce::Viewport {
    public:
      // Override the mouse event methods to forward them to the parent Viewport
      void mouseDown(const juce::MouseEvent& e) override {
        if (juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>()) {
          juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseDown(ep);
        }
        juce::Viewport::mouseDown(e);
      }
      void mouseUp(const juce::MouseEvent& e) override {
        if (juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>()) {
          juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseUp(ep);
        }
        juce::Viewport::mouseUp(e);
      }
      void mouseDrag(const juce::MouseEvent& e) override {
        if (juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>()) {
          juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseDrag(ep);
        }
        juce::Viewport::mouseDrag(e);
      }
      // Override mouseWheelMove to forward events to the parent Viewport
      void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override {
        juce::Viewport* parent = findParentComponentOfClass<juce::Viewport>();
        if (parent != nullptr) {
          juce::MouseEvent ep = juce::MouseEvent(e.source, e.position, e.mods, e.pressure, e.orientation, e.rotation, e.tiltX, e.tiltY, parent, e.originalComponent, e.eventTime, e.mouseDownPosition, e.mouseDownTime, e.getNumberOfClicks(), e.mouseWasDraggedSinceMouseDown());
          parent->mouseWheelMove(ep, wheel);
        }
        juce::Viewport::mouseWheelMove(e, wheel);
      }
    };
    class Table : public juce::Component {
    public:
      void paint(juce::Graphics&) override;
      juce::OwnedArray<juce::OwnedArray<Cell>> cells;
      juce::Array<float> columnwidths;
      juce::Array<float> rowheights;
      juce::Colour bg, bgHeader;
      int cellmargin, cellgap, leftmargin;
    };
    InnerViewport viewport;
    Table table;
  };
  
  class ImageBlock : public Block
  {
  public:
    static bool isImageLine(const juce::String& line);
    void parseImageMarkup(const juce::String& line, FileSource* fileSource);
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
    void resized() override;
  private:
    juce::AttributedString imageMissingMessage;
    juce::Image image;
    int maxWidth;
  };
  
  class ListItem : public Block
  {
  public:
    static bool isListItem(const juce::String& line);
    void parseItemMarkup(const juce::String& line, juce::Font font, int indentPerSpace, int gap);
    float getHeightRequired(float width) override;
    void paint(juce::Graphics&) override;
  private:
    juce::AttributedString attributedString;
    juce::AttributedString label;
    int indent;
    int gap;
  };
  
  // MARK: - Private Variables
  juce::StringPairArray colours;        // colour palette
  juce::Colour bg;                      // background colour
  juce::Colour tableBG, tableBGHeader;  // table background colours
  int tableMargin, tableGap;            // table margins
  int indentPerSpace, labelGap;         // list item indents
  juce::Viewport  viewport;             // a viewport to scroll the content
  juce::Component content;              // a component with the content
  juce::OwnedArray<Block> blocks;       // representation of the document as blocks
  int margin;                           // content margin in pixels
  int iconsize;                         // admonition icon size in pixels
  int admargin;                         // admonition margin in pixels
  int adlinewidth;                      // admonition line width in pixels
  FileSource* fileSource;               // data source for image files, etc.
  juce::Font font;                      // default font for regular text
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarelyMLDisplay)
};
