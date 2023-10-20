/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.
 
 BEGIN_JUCE_PIP_METADATA
 
 name:             BarelyMLDemo
 version:          0.2
 vendor:           Fritz Menzer
 website:          https://mnsp.ch
 description:      A simple demo of the BarelyMLDisplay component, showing how Strings in various formats can be converted to BarelyML and displayed.
 
 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics, juce_gui_basics
 exporters:        ANDROIDSTUDIO, LINUX_MAKE, XCODE_IPHONE, XCODE_MAC
 
 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1
 
 type:             Component
 mainClass:        BarelyMLDemo
 
 END_JUCE_PIP_METADATA
 
 *******************************************************************************/

#pragma once

#include "BarelyML.h"
#include "BarelyML.cpp" // ugly, but works...

#define BarelyML_ID 1
#define Markdown_ID 2
#define DokuWiki_ID 3
#define AsciiDoc_ID 4

using namespace juce;

//==============================================================================
class BarelyMLDemo  : public Component, TextEditor::Listener, ComboBox::Listener
{
public:
  //==============================================================================
  BarelyMLDemo()
  {
    // set up the BarelyMLDisplay
    addAndMakeVisible(display);
    display.setFont(Font("Helvetica Neue", 15.0f, 0));

    // set up the BarelyML TextEditor
    addAndMakeVisible(editor);
    editor.setMultiLine(true);
    editor.setReturnKeyStartsNewLine(true);
    editor.addListener(this);
    editor.setFont(Font(Font::getDefaultMonospacedFontName(), 15.0f, 0));

    // set up the TextEditor for importing other formats
    addChildComponent(importEditor);
    importEditor.setMultiLine(true);
    importEditor.setReturnKeyStartsNewLine(true);
    importEditor.addListener(this);
    importEditor.setFont(Font(Font::getDefaultMonospacedFontName(), 15.0f, 0));

    // set up the format Label
    formatLabel.setText("Markup Format", dontSendNotification);
    addAndMakeVisible(formatLabel);
    
    // set up the format ComboBox
    formatBox.addItem("BarelyML", BarelyML_ID);
    formatBox.addItem("Markdown", Markdown_ID);
    formatBox.addItem("DokuWiki", DokuWiki_ID);
    formatBox.addItem("AsciiDoc", AsciiDoc_ID);
    formatBox.setSelectedId(1);
    formatBox.addListener(this);
    addAndMakeVisible(formatBox);

    setSize (800, 600);
  }
  
  ~BarelyMLDemo() override
  {
  }
  
  // TextEditor::Listener method
  void textEditorTextChanged (TextEditor& editorThatWasChanged) override {
    if (&editorThatWasChanged == &editor) {
      display.setMarkupString(editor.getText());
    }
    if (&editorThatWasChanged == &importEditor) {
      if (formatBox.getSelectedId() == Markdown_ID) {
        editor.setText(BarelyMLDisplay::convertFromMarkdown(importEditor.getText()));
      } else if (formatBox.getSelectedId() == DokuWiki_ID) {
        editor.setText(BarelyMLDisplay::convertFromDokuWiki(importEditor.getText()));
      } else if (formatBox.getSelectedId() == AsciiDoc_ID) {
        editor.setText(BarelyMLDisplay::convertFromAsciiDoc(importEditor.getText()));
      }
    }
  }

  // ComboBox::Listener method
  void comboBoxChanged(ComboBox* box) override {
    if (formatBox.getSelectedId() == 1) {
      importEditor.setVisible(false);
      editor.setEnabled(true);
    } else {
      if (!importEditor.isVisible()) {
        // we're switching from BarelyML to another markup language,
        // i.e. we should put something in the importEditor
        // => let's convert BarelyML to the chosen markup language.
        if (formatBox.getSelectedId() == Markdown_ID) {
          importEditor.setText(BarelyMLDisplay::convertToMarkdown(editor.getText()));
        } else if (formatBox.getSelectedId() == DokuWiki_ID) {
          importEditor.setText(BarelyMLDisplay::convertToDokuWiki(editor.getText()));
        } else if (formatBox.getSelectedId() == AsciiDoc_ID) {
          importEditor.setText(BarelyMLDisplay::convertToAsciiDoc(editor.getText()));
        }
      }
      importEditor.setVisible(true);
      editor.setEnabled(false);
    }
    resized();
  }
  
  //==============================================================================
  void paint (juce::Graphics& g) override
  {
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
  }
  
  void resized() override
  {
    int h = getHeight();
    if (importEditor.isVisible()) { // two editor layout
      int v = getWidth()-40; // 4 gaps of 10 pixels each
      importEditor.setBounds(10, 10, v/3, h-54);
      editor.setBounds(v/3+20, 10, v/3, h-54);
      display.setBounds(2*v/3+30, 10, v-2*v/3, h-20);
    } else {                        // one editor layout
      int v = getWidth()-30; // 3 gaps of 10 pixels each
      editor.setBounds(10, 10, v/2, h-54);
      display.setBounds(v/2+20, 10, v-v/2, h-20);
    }
    formatLabel.setBounds(10, h-34, 120, 24);
    formatBox.setBounds(140, h-34, display.getX()-150, 24);
  }
  
  
private:
  BarelyMLDisplay display;
  TextEditor      editor;
  TextEditor      importEditor;
  Label           formatLabel;
  ComboBox        formatBox;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BarelyMLDemo)
};

