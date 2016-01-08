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
    timescale = new RTNFTimescale(this);

    int timerInterval = (int)(1000/processor->screen_update_factor);

    timescale->setTimebase(timebase);

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

    offset_ = 0.05;
    scale_ = 4.0;

    //initalizing turned on indecies
    for(int i=0;i<128;i++)
        processor->selected_channel_indecides.push_back(1);

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

    font = Font("Default", 16, Font::plain);

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

    // plotting baseline and feedback vectors if in feedback
    if(!rtnfTimer->getIsBaseline()){
        std::cout << "in graph plotting" << std::endl;
        float x1,y1;
        x1 = getWidth()/10;
        y1 = getHeight()/2;
        double dVal = 0.0;
        double mean_ = rtnfTimer->getBaselineMean();;
        double stdv_ = rtnfTimer->getBaselineSTDV();
        double timeCounter_ = 0;

        g.setColour(Colour(128,128,128));

        for(int i = 0; i < baseline_values.size(); ++i){

            dVal = getHeight() * offset_ * ((baseline_values[i] - mean_)/stdv_);
            dVal = truncateDVAL(dVal, y1, y1);
            g.fillRect( (float)(x1 + timeCounter_*scale_), (float)(y1-dVal), (float)scale_, (float)dVal);
            timeCounter_++;
        }

        g.setColour(Colour(0,128,0));
        for(int i = 0; i < feedback_values.size(); ++i){
            dVal = getHeight() * offset_ * ((feedback_values[i] - mean_)/stdv_);
            dVal = truncateDVAL(dVal, y1, y1);
            g.fillRect( (float)(x1 + timeCounter_*scale_), (float)(y1-dVal), (float)scale_, (float)dVal);
            timeCounter_++;
        } 
    }
    g.setColour(Colour(100,100,100));
}

void RTNFDisplayCanvas::updateFeedbackVectors(){
    //processor->globalPower;

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
        std::cout << "avgPowerOnTurnedOnChannels: " << avgPowerOnTurnedOnChannels << std::endl;
        if(rtnfTimer->getTimerCount() < rtnfTimer->getMaxBaseLineLength())

            baseline_values.push_back(avgPowerOnTurnedOnChannels);
        else{
            if(rtnfTimer->getIsBaseline()){
                std::cout << "Starting feedback" << std::endl;
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

bool RTNFDisplayCanvas::getIsUpdateGraph(){
    return isUpdateGraph;
}
void RTNFDisplayCanvas::setIsUpdateGraph(bool status){
    isUpdateGraph = status;
}

double RTNFDisplayCanvas::truncateDVAL(double& dVal_, float& y1_, float& y2_){
    double zeroCheck = 0;
    if( std::isgreater(dVal_, zeroCheck)){
        
        dVal_ = std::min(float(dVal_),y1_/2 -5);
        
    }else{
        dVal_ = std::min(float(-dVal_),y1_/2 -5)*-1;
    }  
    return dVal_;
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
    repaint();
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
    canvas->updateFeedbackVectors();    
    canvas->refresh();
    incrementTimerCount();
}