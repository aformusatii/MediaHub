
// HomeTheaterPC class
class HomeTheaterPC {
    public:
        void mouseMove(int, int);
        void mouseButtonPress(int);
        void mouseButtonRelease(int);
        void keyPress(int);
        void keyRelease(int);
        bool checkIfUp(int, int);
        bool isUp();
        void setUpFlag(bool);
};
