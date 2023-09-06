#pragma once

#include "MurkaTypes.h"
#include "MurkaContext.h"
#include "MurkaView.h"
#include "MurkaInputEventsRegister.h"
#include "MurkaAssets.h"
#include "MurkaLinearLayoutGenerator.h"
#include "MurkaBasicWidgets.h"

using namespace murka;

class M1Checkbox : public murka::View<M1Checkbox> {
public:
    void internalDraw(Murka & m) {

        if (didntInitialiseYet) {
            animatedData = *((bool*)dataToControl) ? 1.0 : 0.0;
            didntInitialiseYet = false;
        }
        
        float animation = A(inside() * enabled);
        
        m.pushStyle();
        m.enableFill();

        if (showCircleWithText) {
                m.setColor(100 + 110 * enabled + 30 * animation, 220);
            m.drawCircle(getSize().y / 2, getSize().y / 2, getSize().y / 2);
            m.setColor(40 + 20 * !enabled, 255);
            m.drawCircle(getSize().y / 2, getSize().y / 2, getSize().y / 2 - 2);
            
            m.setColor(100 + 110 * enabled + 30 * animation, 220);
            
            animatedData = A(*((bool*)dataToControl));
            if (!useButtonMode) {
                m.drawCircle(getSize().y / 2, getSize().y / 2, 4 * animatedData);
            }
        }

        m.setColor(100 + 110 * enabled + 30 * animation, 220);
        m.setFontFromRawData(PLUGIN_FONT, BINARYDATA_FONT, BINARYDATA_FONT_SIZE, fontSize);
        m.prepare<murka::Label>({shape.size.y + 3, 3, 150, shape.size.y + 5}).text(label).draw();
        m.disableFill();
        m.popStyle();

        // Action
        if ((mouseDownPressed(0)) && (inside()) && enabled) {
            if (useButtonMode) {
                changed = true;
            }
            else {
                *((bool*)dataToControl) = !*((bool*)dataToControl);
                changed = true;
            }
        }
        else {
            changed = false;
        }
    }
    
    float animatedData = 0;
    bool didntInitialiseYet = true;
    bool changed = false;
    bool checked = false; // TODO: implement a way to uncheck/check button fill
    std::string label;
    double fontSize = 10;
    bool* dataToControl = nullptr;
    bool showCircleWithText = true;
    bool useButtonMode = false;
    
    M1Checkbox & controlling(bool* dataPointer) {
        dataToControl = dataPointer;
        return *this;
    }
    
    M1Checkbox & withLabel(std::string label_) {
        label = label_;
        return *this;
    }

    M1Checkbox & showCircle(bool show) {
        showCircleWithText = show;
        return *this;
    }

    M1Checkbox & buttonMode(bool use) {
        useButtonMode = use;
        return *this;
    }

    M1Checkbox & withFontSize(double fontSize_) {
        fontSize = fontSize_;
        return *this;
    }

    MURKA_PARAMETER(M1Checkbox, // class name
                    bool, // parameter type
                    enabled, // parameter variable name
                    enable, // setter
                    true // default
    )
    
};
