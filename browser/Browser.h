#include <gtk/gtk.h>

struct MouseCoordinates {
    int x;
    int y;
    bool jump;
};

// Browser class
class Browser {
    private:
       Browser(){
           mouseContinuousMove = false;
           preventDefaultEvents = FALSE;
           afterInit=NULL;
           closeWindow=NULL;
           mouseMove=NULL;
           mouseButtonPress=NULL;
           mouseButtonRelease=NULL;
           keyPress=NULL;keyRelease=NULL;
           receivedUIEvent=NULL;
       };  // Private so that it can  not be called

       Browser(Browser const&){
           preventDefaultEvents = FALSE;
           mouseContinuousMove = false;
           afterInit=NULL;
           closeWindow=NULL;
           mouseMove=NULL;
           mouseButtonPress=NULL;
           mouseButtonRelease=NULL;
           keyPress=NULL;
           keyRelease=NULL;
           receivedUIEvent=NULL;
       }; // copy constructor is private

       bool mouseContinuousMove;
       gboolean preventDefaultEvents;
       static Browser* browserInstance;

    public:
        static Browser* getInstance();
        int initBrowser(int argc, char** argv);

        void setMouseContinuousMove(bool);
        bool isMouseContinuousMove();

        void setPreventDefaultEvents(bool);
        gboolean isPreventDefaultEvents();

        /*  External Event Handlers */
        void (*afterInit)(MouseCoordinates);
        void (*closeWindow)(void);
        void (*mouseMove)(MouseCoordinates);
        void (*mouseButtonPress)(int);
        void (*mouseButtonRelease)(int);
        void (*keyPress)(int);
        void (*keyRelease)(int);
        void (*receivedUIEvent)(char *);
};
