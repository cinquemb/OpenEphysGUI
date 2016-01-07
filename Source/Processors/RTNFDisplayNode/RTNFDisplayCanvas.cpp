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


#include "RTNFDisplayCanvas.h"

RTNFDisplayCanvas::RTNFDisplayCanvas(RTNFDisplayNode* processor_) : processor(processor_), timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f)
{
    viewport = new RTNFViewport(this);
    rtnfDisplay = new RTNFDisplay(this, viewport);
    timescale = new RTNFTimescale(this);


    int timerInterval = (int)(1000/processor->screen_update_factor);

    timescale->setTimebase(timebase);

    

    viewport->setViewedComponent(rtnfDisplay, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();

    addAndMakeVisible(viewport);
    addAndMakeVisible(timescale);

    timebaseSelection = new ComboBox("Timebase");
    timebaseSelection->addItemList(timebases, 1);
    timebaseSelection->setSelectedId(selectedTimebase, sendNotification);
    timebaseSelection->setEditableText(true);
    timebaseSelection->addListener(this);
    addAndMakeVisible(timebaseSelection);

    rtnfTimer = new RTNFTimer(this);
    rtnfTimer->startTimer(timerInterval);

}

RTNFDisplayCanvas::~RTNFDisplayCanvas(){
}

void RTNFDisplayCanvas::paint(Graphics& g){
    //std::cout << "Painting" << std::endl;
    g.setColour(Colour(0,18,43)); //background color
    g.fillRect(0, 0, getWidth(), getHeight());

    g.setGradientFill(ColourGradient(Colour(50,50,50),0,0,
                                     Colour(25,25,25),0,30,
                                     false));

    g.fillRect(0, 0, getWidth()-scrollBarThickness, 30);

    g.setColour(Colours::black);

    g.drawLine(0,30,getWidth()-scrollBarThickness,30);

    g.setColour(Colour(25,25,60)); // timing grid color

    int w = getWidth()-scrollBarThickness-leftmargin;

    for (int i = 0; i < 10; i++)
    {
        if (i == 5 || i == 0)
            g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,3.0f);
        else
            g.drawLine(w/10*i+leftmargin,0,w/10*i+leftmargin,getHeight()-60,1.0f);
    }

    g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);

    g.setFont(Font("Default", 16, Font::plain));

    g.setColour(Colour(100,100,100));

    //g.drawText("Voltage range ("+ rangeUnits[selectedChannelType] +")",5,getHeight()-55,300,20,Justification::left, false);
    g.drawText("Time (s)",175,getHeight()-55,300,20,Justification::left, false);
    //g.drawText("Spread (px)",345,getHeight()-55,300,20,Justification::left, false);
    //g.drawText("Color grouping",620,getHeight()-55,300,20,Justification::left, false);

    //g.drawText(typeNames[selectedChannelType],110,getHeight()-30,50,20,Justification::centredLeft,false);

    //g.drawText("Event disp.",500,getHeight()-55,300,20,Justification::left, false);
}

void RTNFDisplayCanvas::updateFeedbackVectors(){
    //processor->globalPower;

    std::cout << "updateFeedbackVectors is being called" << std::endl;

    //pull power samples from turned on channels and pushback onto appropriate vector
    double tempPowerIndexSum = 0.0;
    int turnedOnChannels = 0;
    for(int i =0; i< processor->current_channel_power.size(); ++i){
        if(processor->selected_channel_indecides[i] == 1){
            turnedOnChannels++;
            tempPowerIndexSum += processor->current_channel_power[i]/processor->screen_update_factor;
        }
    }

    if(turnedOnChannels > 0){
        double avgPowerOnTurnedOnChannels = tempPowerIndexSum/turnedOnChannels;
        if(rtnfTimer->getTimerCount() < rtnfTimer->getMaxBaseLineLength())
            baseline_values.push_back(avgPowerOnTurnedOnChannels);
        else{
            if(rtnfTimer->getIsBaseline()){
                double tempSum = 0.0;
                int baseline_length = baseline_values.size();
                for(auto iterM : baseline_values)
                    tempSum += iterM;
                rtnfTimer->setBaselineMean(tempSum/baseline_length);
                
                tempSum = 0.0;
                double baseline_mean = rtnfTimer->getBaselineMean();
                for(auto iterS: baseline_values)
                    tempSum += std::pow((iterS - baseline_mean), 2);

                rtnfTimer->setBaselineSTDV(std::sqrt(tempSum/(baseline_length-1)));
                rtnfTimer->setIsBaseline(false);
            }
            feedback_values.push_back(avgPowerOnTurnedOnChannels);
        }
    }
}


void RTNFDisplayCanvas::resized(){

}

void RTNFDisplayCanvas::saveVisualizerParameters(XmlElement* xml){

}

void RTNFDisplayCanvas::loadVisualizerParameters(XmlElement* xml){

}

void RTNFDisplayCanvas::updateScreenBuffer(){

}

void RTNFDisplayCanvas::update(){

}

void RTNFDisplayCanvas::beginAnimation(){

}

void RTNFDisplayCanvas::endAnimation(){

}

void RTNFDisplayCanvas::setParameter(int, float){}

void RTNFDisplayCanvas::refreshState(){}
void RTNFDisplayCanvas::comboBoxChanged(ComboBox* cb){}
void RTNFDisplayCanvas::buttonClicked(Button* button){}

void RTNFDisplayCanvas::refresh()
{

    updateScreenBuffer();

    //RTNFDisplay->refresh(); // redraws only the new part of the screen buffer

    //getPeer()->performAnyPendingRepaintsNow();

}

bool RTNFDisplayCanvas::keyPressed(const KeyPress& key)
{
    if (key.getKeyCode() == key.spaceKey)
    {
        pauseButton->setToggleState(!pauseButton->getToggleState(), sendNotification);
        return true;
    }

    return false;
}

bool RTNFDisplayCanvas::keyPressed(const KeyPress& key, Component* orig)
{
    if (getTopLevelComponent() == orig && isVisible())
    {
        return keyPressed(key);
    }
    return false;
}

int RTNFDisplayCanvas::getRangeStep(ChannelType type)
{
    return 1;
}

void RTNFDisplayCanvas::setSpreadSelection(int spread, bool canvasMustUpdate)
{
    if (canvasMustUpdate)
    {
        spreadSelection->setText(String(spread),sendNotification);
    }
    else
    {
        spreadSelection->setText(String(spread),dontSendNotification);
        selectedSpread=spreadSelection->getSelectedId();
        selectedSpreadValue=spreadSelection->getText();

        repaint();
        refresh();
    }
}


ChannelType RTNFDisplayCanvas::getChannelType(int n)
{
    if (n < processor->getNumInputs())
        return processor->channels[n]->getType();
    else
        return HEADSTAGE_CHANNEL;
}

ChannelType RTNFDisplayCanvas::getSelectedType()
{
    return selectedChannelType;
}

// Rtnf Viewport -------------------------------------------

RTNFViewport::RTNFViewport(RTNFDisplayCanvas *canvas)
    : Viewport()
{
    this->canvas = canvas;
}

void RTNFViewport::visibleAreaChanged(const Rectangle<int>& newVisibleArea)
{
    canvas->fullredraw = true;
    canvas->refresh();
}

// Rtnf Display -------------------------------------------

RTNFDisplay::RTNFDisplay(RTNFDisplayCanvas* c, Viewport* v) :
    singleChan(-1), canvas(c), viewport(v)
{
    totalHeight = 0;
    colorGrouping=1;

    range[0] = 1000;
    range[1] = 500;
    range[2] = 500000;

    addMouseListener(this, true);

    //hand-built palette
    channelColours.add(Colour(224,185,36));
    channelColours.add(Colour(214,210,182));
    channelColours.add(Colour(243,119,33));
    channelColours.add(Colour(186,157,168));
    channelColours.add(Colour(237,37,36));
    channelColours.add(Colour(179,122,79));
    channelColours.add(Colour(217,46,171));
    channelColours.add(Colour(217, 139,196));
    channelColours.add(Colour(101,31,255));
    channelColours.add(Colour(141,111,181));
    channelColours.add(Colour(48,117,255));
    channelColours.add(Colour(184,198,224));
    channelColours.add(Colour(116,227,156));
    channelColours.add(Colour(150,158,155));
    channelColours.add(Colour(82,173,0));
    channelColours.add(Colour(125,99,32));

    isPaused=false;

}

RTNFDisplay::~RTNFDisplay()
{
    deleteAllChildren();
}

int RTNFDisplay::getNumChannels()
{
    return numChans;
}



int RTNFDisplay::getColorGrouping()
{
    return colorGrouping;
}

void RTNFDisplay::setColorGrouping(int i)
{
    colorGrouping=i;
    setColors(); // so that channel colors get re-assigned

}


void RTNFDisplay::setNumChannels(int numChannels)
{
    numChans = numChannels;

    deleteAllChildren();

    channels.clear();
    channelInfo.clear();

    totalHeight = 0;

    setColors();

    //std::cout << "TOTAL HEIGHT = " << totalHeight << std::endl;

    // // this doesn't seem to do anything:
    //canvas->fullredraw = true;
    //refresh();

}

void RTNFDisplay::setColors()
{
}


int RTNFDisplay::getTotalHeight()
{
    return totalHeight;
}

void RTNFDisplay::resized()
{
    // no need to resize because only plotting one graph, maybe in future we'll want to plot data streams processed differently

}

void RTNFDisplay::paint(Graphics& g)
{

}

void RTNFDisplay::refresh()
{


    int topBorder = viewport->getViewPositionY();
    int bottomBorder = viewport->getViewHeight() + topBorder;

    canvas->fullredraw = false;
}

void RTNFDisplay::setRange(float r, ChannelType type)
{
    range[type] = r;
    canvas->fullredraw = true; //issue full redraw
}

int RTNFDisplay::getRange()
{
    return getRange(canvas->getSelectedType());
}

int RTNFDisplay::getRange(ChannelType type)
{
    return 0;
}


void RTNFDisplay::setChannelHeight(int r, bool resetSingle)
{

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setChannelHeight(r);
        channelInfo[i]->setChannelHeight(r);
    }
    if (resetSingle && singleChan != -1)
    {
        setSize(getWidth(),numChans*getChannelHeight());
        viewport->setScrollBarsShown(true,false);
        viewport->setViewPosition(Point<int>(0,singleChan*r));
        singleChan = -1;
    }

    resized();

}

void RTNFDisplay::setInputInverted(bool isInverted)
{

    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setInputInverted(isInverted);
    }

    resized();

}

void RTNFDisplay::setDrawMethod(bool isDrawMethod)
{
    for (int i = 0; i < numChans; i++)
    {
        channels[i]->setDrawMethod(isDrawMethod);
    }
    resized();

}


int RTNFDisplay::getChannelHeight()
{
    return 0;
}



void RTNFDisplay::mouseWheelMove(const MouseEvent&  e, const MouseWheelDetails&   wheel)
{

    //std::cout << "Mouse wheel " <<  e.mods.isCommandDown() << "  " << wheel.deltaY << std::endl;
    //TODO Changing ranges with the wheel is currently broken. With multiple ranges, most
    //of the wheel range code needs updating
    if (e.mods.isCommandDown())  // CTRL + scroll wheel -> change channel spacing
    {
        int h = getChannelHeight();
        int hdiff=0;
        
        std::cout << wheel.deltaY << std::endl;
        
        if (wheel.deltaY > 0)
        {
            hdiff = 2;
        }
        else
        {
            if (h > 5)
                hdiff = -2;
        }

        if (abs(h) > 100) // accelerate scrolling for large ranges
            hdiff *= 3;

        setChannelHeight(h+hdiff);
        int oldX=viewport->getViewPositionX();
        int oldY=viewport->getViewPositionY();

        setBounds(0,0,getWidth()-0, getChannelHeight()*canvas->nChans); // update height so that the scrollbar is correct

        int mouseY=e.getMouseDownY(); // should be y pos relative to inner viewport (0,0)
        int scrollBy = (mouseY/h)*hdiff*2;// compensate for motion of point under current mouse position
        viewport->setViewPosition(oldX,oldY+scrollBy); // set back to previous position plus offset

        canvas->setSpreadSelection(h+hdiff); // update combobox

    }
    else
    {
        if (e.mods.isShiftDown())  // SHIFT + scroll wheel -> change channel range
        {
            int h = getRange();
            int step = canvas->getRangeStep(canvas->getSelectedType());
            
            std::cout << wheel.deltaY << std::endl;
            
            if (wheel.deltaY > 0)
            {
                setRange(h+step,canvas->getSelectedType());
            }
            else
            {
                if (h > step+1)
                    setRange(h-step,canvas->getSelectedType());
            }

            //canvas->setRangeSelection(h); // update combobox

        }
        else    // just scroll
        {
            //  passes the event up to the viewport so the screen scrolls
            if (viewport != nullptr && e.eventComponent == this) // passes only if it's not a listening event
                viewport->mouseWheelMove(e.getEventRelativeTo(canvas), wheel);

        }
    }

    canvas->fullredraw = true;//issue full redraw
    refresh();

}

void RTNFDisplay::toggleSingleChannel(int chan)
{
    setChannelHeight(canvas->getChannelHeight());
}

bool RTNFDisplay::getSingleChannelState()
{
    if (singleChan < 0) return false;
    else return true;
}


void RTNFDisplay::mouseDown(const MouseEvent& event)
{
    //int y = event.getMouseDownY(); //relative to each channel pos
    MouseEvent canvasevent = event.getEventRelativeTo(viewport);
    int y = canvasevent.getMouseDownY() + viewport->getViewPositionY(); // need to account for scrolling

    canvas->fullredraw = true;//issue full redraw

    refresh();

}


bool RTNFDisplay::setEventDisplayState(int ch, bool state)
{
    eventDisplayEnabled[ch] = state;
    return eventDisplayEnabled[ch];
}


bool RTNFDisplay::getEventDisplayState(int ch)
{
    return eventDisplayEnabled[ch];
}

void RTNFDisplay::enableChannel(bool state, int chan)
{
}

void RTNFDisplay::setEnabledState(bool state, int chan)
{
}

bool RTNFDisplay::getEnabledState(int chan)
{
    return true;
}

// Rtnf Timescale -------------------------------------------


RTNFTimescale::RTNFTimescale(RTNFDisplayCanvas* c) : canvas(c)
{

    font = Font("Default", 16, Font::plain);
}

RTNFTimescale::~RTNFTimescale()
{

}

void RTNFTimescale::paint(Graphics& g)
{
    g.setFont(font);

    g.setColour(Colour(100,100,100));

    g.drawText("ms:",5,0,100,getHeight(),Justification::left, false);

    for (int i = 1; i < 10; i++)
    {
        if (i == 5)
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),3.0f);
        else
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),1.0f);

        g.drawText(labels[i-1],getWidth()/10*i+3,0,100,getHeight(),Justification::left, false);
    }
}

void RTNFTimescale::setTimebase(float t)
{
    timebase = t;

    labels.clear();

    for (float i = 1.0f; i < 10.0; i++)
    {
        String labelString = String(timebase/10.0f*1000.0f*i);

        labels.add(labelString.substring(0,6));
    }

    repaint();

}

RTNFTimer::RTNFTimer(RTNFDisplayCanvas* c){
    resetTimerCount();
    setBaselineMean(0.0);
    setBaselineSTDV(0.0);
    setIsBaseline(true);
    setMaxBaseLineLength(30);
    this->canvas = c;
}

RTNFTimer::~RTNFTimer(){
    stopTimer();
}

void RTNFTimer::incrementTimerCount(){++timerCount;}
void RTNFTimer::resetTimerCount(){timerCount = 0;}
int RTNFTimer::getTimerCount(){return timerCount;}

void RTNFTimer::setBaselineMean(double mean){baselineMean = mean;}
void RTNFTimer::setBaselineSTDV(double stdv){baselineSTDV = stdv;}
void RTNFTimer::setIsBaseline(bool _isBaseLine){isBaseLine = _isBaseLine;}
void RTNFTimer::setMaxBaseLineLength(int isBaseLine){max_baseline_length = isBaseLine;}


double RTNFTimer::getBaselineMean(){return baselineMean;}
double RTNFTimer::getBaselineSTDV(){return baselineSTDV;}
bool RTNFTimer::getIsBaseline(){return isBaseLine;}
int RTNFTimer::getMaxBaseLineLength(){return max_baseline_length;}

void RTNFTimer::hiResTimerCallback(){
    std::cout << "here" << std::endl;
    canvas->updateFeedbackVectors();
    incrementTimerCount();
}