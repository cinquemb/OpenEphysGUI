/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Visualization/Visualizer.h"
#include "RTNFDisplayNode.h"

#ifndef RTNFDISPLAYCANVAS_H_INCLUDED
#define RTNFDISPLAYCANVAS_H_INCLUDED

class RTNFDisplayNode;

class RTNFTimescale;
class RTNFDisplay;
class RTNFDisplayInfo;
class RTNFEventDisplayInterface;
class RTNFViewport;

class RTNFDisplayCanvas : public Visualizer,
    public ComboBox::Listener,
    public Button::Listener,
    public KeyListener
{
    public:
        RTNFDisplayCanvas(RTNFDisplayNode* processor_);
        ~RTNFDisplayCanvas();

        void updateFeedback();
        
        std::vector<double> baseline_values;
        std::vector<double> feedback_values;

        void beginAnimation();
        void endAnimation();

        void refreshState();
        void update();

        void setParameter(int, float);
        void setParameter(int, int, int, float) {}

        void setRangeSelection(float range, bool canvasMustUpdate = false); // set range selection combo box to correct value if it has been changed by scolling etc.
        void setSpreadSelection(int spread, bool canvasMustUpdate = false); // set spread selection combo box to correct value if it has been changed by scolling etc.

        void paint(Graphics& g);

        void refresh();

        void resized();

        int getChannelHeight();

        int getNumChannels();
        bool getInputInvertedState();
        bool getDrawMethodState();

        const float getXCoord(int chan, int samp);
        const float getYCoord(int chan, int samp);

        const float getYCoordMin(int chan, int samp);
        const float getYCoordMean(int chan, int samp);
        const float getYCoordMax(int chan, int samp);

        Array<int> screenBufferIndex;
        Array<int> lastScreenBufferIndex;

        void comboBoxChanged(ComboBox* cb);
        void buttonClicked(Button* button);

        void saveVisualizerParameters(XmlElement* xml);
        void loadVisualizerParameters(XmlElement* xml);

        bool keyPressed(const KeyPress& key);
        bool keyPressed(const KeyPress& key, Component* orig);

        ChannelType getChannelType(int n);
        ChannelType getSelectedType();
        String getTypeName(ChannelType type);
        int getRangeStep(ChannelType type);

        void setSelectedType(ChannelType type, bool toggleButton = true);

        //void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart);

        bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw, there is a similar switch for ach ch display;
        static const int leftmargin=50; // left margin for lfp plots (so the ch number text doesnt overlap)

        Array<bool> isChannelEnabled;

        int nChans;

    private:
        Array<float> sampleRate;
        float timebase;
        float displayGain;
        float timeOffset;
        //int spread ; // vertical spacing between channels


        static const int MAX_N_CHAN = 2048;  // maximum number of channels
        static const int MAX_N_SAMP = 5000; // maximum display size in pixels
        //float waves[MAX_N_CHAN][MAX_N_SAMP*2]; // we need an x and y point for each sample

        RTNFDisplayNode* processor;
        AudioSampleBuffer* displayBuffer; // sample wise data buffer for display
        AudioSampleBuffer* screenBuffer; // subsampled buffer- one int per pixel

        //'define 3 buffers for min mean and max for better plotting of spikes
        // not pretty, but 'AudioSampleBuffer works only for channels X samples
        AudioSampleBuffer* screenBufferMin; // like screenBuffer but holds min/mean/max values per pixel
        AudioSampleBuffer* screenBufferMean; // like screenBuffer but holds min/mean/max values per pixel
        AudioSampleBuffer* screenBufferMax; // like screenBuffer but holds min/mean/max values per pixel

        MidiBuffer* eventBuffer;

        ScopedPointer<RTNFTimescale> timescale;
        ScopedPointer<RTNFDisplay> rtnfDisplay;
        ScopedPointer<RTNFViewport> viewport;

        ScopedPointer<ComboBox> timebaseSelection;
        ScopedPointer<ComboBox> rangeSelection;
        ScopedPointer<ComboBox> spreadSelection;
        ScopedPointer<ComboBox> colorGroupingSelection;
        ScopedPointer<UtilityButton> invertInputButton;
        ScopedPointer<UtilityButton> drawMethodButton;
        ScopedPointer<UtilityButton> pauseButton;
        OwnedArray<UtilityButton> typeButtons;

        StringArray timebases;
        StringArray spreads; // option for vertical spacing between channels
        StringArray colorGroupings; // option for coloring every N channels the same

        ChannelType selectedChannelType;
        StringArray rangeUnits;
        StringArray typeNames;

        int selectedSpread;
        String selectedSpreadValue;

        int selectedTimebase;
        String selectedTimebaseValue;

        OwnedArray<RTNFEventDisplayInterface> eventDisplayInterfaces;

        void refreshScreenBuffer();
        void updateScreenBuffer();

        Array<int> displayBufferIndex;
        int displayBufferSize;

        int scrollBarThickness;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RTNFDisplayCanvas);
};

class RTNFTimescale : public Component
{
public:
    RTNFTimescale(RTNFDisplayCanvas*);
    ~RTNFTimescale();

    void paint(Graphics& g);

    void setTimebase(float t);

private:

    RTNFDisplayCanvas* canvas;

    float timebase;

    Font font;

    StringArray labels;

};

class RTNFDisplay : public Component
{
public:
    RTNFDisplay(RTNFDisplayCanvas*, Viewport*);
    ~RTNFDisplay();

    void setNumChannels(int numChannels);
    int getNumChannels();

    int getTotalHeight();

    void paint(Graphics& g);

    void refresh();

    void resized();

    void mouseDown(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent&  event, const MouseWheelDetails&   wheel) ;


    void setRange(float range, ChannelType type);
    
    //Withouth parameters returns selected type
    int getRange();
    int getRange(ChannelType type);

    void setChannelHeight(int r, bool resetSingle = true);
    int getChannelHeight();
    void setInputInverted(bool);
    void setDrawMethod(bool);

    void setColors();

    bool setEventDisplayState(int ch, bool state);
    bool getEventDisplayState(int ch);

    int getColorGrouping();
    void setColorGrouping(int i);

    void setEnabledState(bool, int);
    bool getEnabledState(int);
    void enableChannel(bool, int);

    bool getSingleChannelState();

    Array<Colour> channelColours;

    Array<RTNFDisplay*> channels;
    Array<RTNFDisplay*> channelInfo;

    bool eventDisplayEnabled[8];
    bool isPaused; // simple pause function, skips screen bufer updates

private:
    void toggleSingleChannel(int chan);
    int singleChan;
    Array<bool> savedChannelState;

    int numChans;

    int totalHeight;

    int colorGrouping;

    RTNFDisplayCanvas* canvas;
    Viewport* viewport;

    float range[3];


};

class RTNFDisplayInfo : public RTNFDisplay,
    public Button::Listener
{
public:
    RTNFDisplayInfo(RTNFDisplayCanvas*, RTNFDisplay*);

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void resized();

    void setEnabledState(bool);
    void updateType();

private:

    ScopedPointer<UtilityButton> enableButton;

};

class RTNFEventDisplayInterface : public Component,
    public Button::Listener
{
public:
    RTNFEventDisplayInterface(RTNFDisplay*, RTNFDisplayCanvas*, int chNum);
    ~RTNFEventDisplayInterface();

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void checkEnabledState();

    bool isEnabled;

private:

    int channelNumber;

    RTNFDisplay* display;
    RTNFDisplayCanvas* canvas;

    ScopedPointer<UtilityButton> chButton;

};

class RTNFViewport : public Viewport
{
public:
    RTNFViewport(RTNFDisplayCanvas* canvas);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);

private:
    RTNFDisplayCanvas* canvas;
};

#endif  // RTNFDISPLAYCANVAS_H_INCLUDED
