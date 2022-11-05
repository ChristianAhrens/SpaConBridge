<a name="architectureoverview"/>

## App architecture

<div class="mermaid">graph TB
    subgraph SpaConBridge
      direction LR

      subgraph PagedUI
        direction TB

        pageMgr[PageManager]

        sndObjTablePage[SoundobjectTablePage]
        multiSndObjPage[MultiSoundobjectPage]
        matrixIOPage[MartixIOPage]
        scnPage[ScenesPage]
        enSpacePage[EnSpacePage]
        statsPage[StatisticsPage]
        settingsPage[SettingsPage]

        pageMgr <--> sndObjTablePage
        pageMgr <--> multiSndObjPage
        pageMgr <--> matrixIOPage
        pageMgr <--> scnPage
        pageMgr <--> enSpacePage
        pageMgr <--> statsPage
        pageMgr <--> settingsPage
      end

      ctrl[Controller]
      pageMgr <--> ctrl
      ctrl <--> objHndl

      subgraph ProtocolBridge
        
        subgraph ProcessingEngine
          direction TB

          objHndl[ObjectDataHandling]
          p1(DS100:OSCProtocolProcessor)
          p2(GenericOSC:OSCProtocolProcessor)
          p3(d&bSoundscapePlugin:OSCProtocolProcessor)
          p4(YamahaOSC:OSCProtocolProcessor)
          p5(ADMOSC:OSCProtocolProcessor)
          p6(MIDI:MIDIProtocolProcessor)
          p7(BlackTrax:RTTrPMProtocolProcessor)

          objHndl <--> p1
          objHndl <--> p2
          objHndl <--> p3
          objHndl <--> p4
          objHndl <--> p5
          objHndl <--> p6
          objHndl <--> p7
        end
      end
    end

    classDef green fill:#9f6,stroke:#333,stroke-width:2px;
    classDef orange fill:#f96,stroke:#333,stroke-width:2px;
    classDef lightgrey fill:#bbb,stroke:#333,stroke-width:2px;
    classDef grey fill:#999,stroke:#333,stroke-width:2px;
    classDef framedWhite fill:#fff,stroke:#333,stroke-width:2px;
    class pagedUI,ctrl green
    class objHndl,p1,p2,p3,p4,p5,p6,p7 orange
    class ProcessingEngine grey
    class ProtocolBridge lightgrey
    class SpaConBridge framedWhite
    class PagedUI lightgrey
</div>

<div class="mermaid">classDiagram
  Animal <|-- Duck
  Animal <|-- Fish
  Animal <|-- Zebra
  Animal : +int age
  Animal : +String gender
  Animal: +isMammal()
  Animal: +mate()
  class Duck{
      +String beakColor
      +swim()
      +quack()
  }
  class Fish{
      +Fins fins
      -int sizeInFeet
      -canEat()
  }
  class Zebra{
      +bool is_wild
      +run()
  }

  class Fins{
    +int count
  }

</div>

<div class="mermaid">C4Context
  title SpaConBridge Architecture
    Boundary(SpaConBridge, "SpaConBridge")
    {
      Boundary(PagedUI, "PagedUI")
      {
        Component(pageMgr, "PageManager", "")

        Component(sndObjTablePage, "SoundobjectTablePage", "")
        Component(multiSndObjPage, "MultiSoundobjectPage", "")
        Component(matrixIOPage, "MartixIOPage", "")
        Component(scnPage, "ScenesPage", "")
        Component(enSpacePage, "EnSpacePage", "")
        Component(statsPage, "StatisticsPage", "")
        Component(settingsPage, "SettingsPage", "")

        BiRel(pageMgr, sndObjTablePage, "")
        BiRel(pageMgr, multiSndObjPage, "")
        BiRel(pageMgr, matrixIOPage, "")
        BiRel(pageMgr, scnPage, "")
        BiRel(pageMgr, enSpacePage, "")
        BiRel(pageMgr, statsPage, "")
        BiRel(pageMgr, settingsPage, "")
      }

      Component(ctrl, "Controller", "")
      BiRel(pageMgr, ctrl, "")
      BiRel(ctrl, objHndl, "")

      Boundary(ProtocolBridge, "ProtocolBridge")
      {
        Boundary(ProcessingEngine, "ProcessingEngine")
        {
          Component(objHndl, "objectDataHandling", "")
          Component(p1, "DS100:OSCProtocolProcessor", "")
          Component(p2, "GenericOSC:OSCProtocolProcessor", "")
          Component(p3, "d&bSoundscapePlugin:OSCProtocolProcessor", "")
          Component(p4, "YamahaOSC:OSCProtocolProcessor", "")
          Component(p5, "ADMOSC:OSCProtocolProcessor", "")
          Component(p6, "MIDI:MIDIProtocolProcessor", "")
          Component(p7, "BlackTrax:RTTrPMProtocolProcessor", "")

          BiRel(objHndl, p1, "")
          BiRel(objHndl, p2, "")
          BiRel(objHndl, p3, "")
          BiRel(objHndl, p4, "")
          BiRel(objHndl, p5, "")
          BiRel(objHndl, p6, "")
          BiRel(objHndl, p7, "")
        }
      }
    }
</div>