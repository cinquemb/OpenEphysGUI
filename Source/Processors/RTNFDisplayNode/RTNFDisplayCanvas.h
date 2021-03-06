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
class RTNFDisplayInfo;
class RTNFEventDisplayInterface;
class RTNFViewport;
class RTNFTimer;

class RTNFDisplayCanvas : public Visualizer,
    public ComboBox::Listener,
    public Button::Listener,
    public KeyListener
{
    public:
        RTNFDisplayCanvas(RTNFDisplayNode* processor_);
        ~RTNFDisplayCanvas();

        void updateFeedbackVectors();
        
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

        bool getIsUpdateGraph();
        void setIsUpdateGraph(bool status);
        double truncateDVAL(double& dVal_, float& y1_, float& y2_);

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
        ScopedPointer<RTNFViewport> viewport;
        ScopedPointer<RTNFTimer> rtnfTimer;

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

        Font font;
        StringArray labels;
        bool isUpdateGraph;
        double offset_;
        double scale_;

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

class RTNFViewport : public Viewport
{
public:
    RTNFViewport(RTNFDisplayCanvas* canvas);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);

private:
    RTNFDisplayCanvas* canvas;
};

class RTNFTimer : public HighResolutionTimer
{
    public:
        RTNFTimer(RTNFDisplayCanvas* canvas);
        ~RTNFTimer();
        void hiResTimerCallback();

        void incrementTimerCount();
        void resetTimerCount();
        int getTimerCount();

        void setBaselineMean(double mean);
        void setBaselineSTDV(double stdv);
        void setIsBaseline(bool _isBaseLine);
        void setMaxBaseLineLength(int isBaseLine);


        double getBaselineMean();
        double getBaselineSTDV();
        bool getIsBaseline();
        int getMaxBaseLineLength();


    private:
        RTNFDisplayCanvas* canvas;
        int timerCount;
        double baselineMean;
        double baselineSTDV;
        bool isBaseLine;
        int max_baseline_length;
};

#endif  // RTNFDISPLAYCANVAS_H_INCLUDED
