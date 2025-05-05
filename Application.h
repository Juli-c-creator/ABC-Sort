#ifndef APPLICATION_H
#define APPLICATION_H

#include <FL/Enumerations.H>
#include <bobcat_ui/all.h>
#include <bobcat_ui/bobcat_ui.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

class Application : public bobcat::Application_ {
    bobcat::Window* window;

    bobcat::TextBox* titleTextbox;

    //variables
    std::string currentScreen;
    int roundNumber;

    //shared screen variables
    bobcat::Button* returnToMainMenuButton;

    //main menu screen variables
    bobcat::Button* playGameButton;
    bobcat::Button* instructionsButton;
    bobcat::Button* statsButton;
    bobcat::Button* quitGameButton;

    //instructions screen variables
    bobcat::TextBox* instructionsTextBox;

    //statistics screen variables
    bobcat::TextBox* statisticsRoundTextbox;
    bobcat::TextBox* statisticsOutcomesTextbox;
    bobcat::Button* playAgainButton;

    //play game screen variables
    bobcat::Button* giveUpButton;
    bobcat::TextBox* gameStateTextbox;
    bobcat::TextBox* spinningLetterTextbox;
    bobcat::Button* slotButtons[6];

    //game logic variables
    std::vector<char> selectedLetters;
    char placedLetters[6];
    bool slotUsed[6];
    char currentLetter;
    bool isSpinning; // whether letter is spinning
    int spinCount = 0;

    //write data to file
    void writeDataToFile(int roundNumber, std::string outcome) {
        std::ofstream file("statistics.csv", std::ios::app);
        if (file.is_open()) {
            file << roundNumber << "," << outcome << std::endl;
        }
    }

    static void spinStepCallback(void* userdata){
        Application* app = static_cast<Application*>(userdata);
        app->spinStep();
    }

    //load last round number
    void loadLastRoundNumber() {
        std::ifstream file("statistics.csv");
        std::string line, lastLine;
        while (getline(file, line)) {
            if (!line.empty()) lastLine = line;
        }
        if (!lastLine.empty()) {
            size_t commaPos = lastLine.find(',');
            if (commaPos != std::string::npos) {
                try {
                    roundNumber = std::stoi(lastLine.substr(0, commaPos));
                } catch (...) {
                    roundNumber = 0;
                }
            }
        } else {
            roundNumber = 0;
        }
    }

    //read stats from file and only displays most recent 10 rounds
    void readStatsFromFile(){
        std::ifstream file("statistics.csv");
        std::string line;
        std::vector<std::string> lines;

        while(getline(file, line)){
            lines.push_back(line);
        }

        statisticsRoundTextbox->label("Round\n");
        statisticsOutcomesTextbox->label("Outcome\n");

        int start = std::max(0, static_cast<int>(lines.size()) - 10);
        for(int i = start; i < lines.size(); ++i){
            size_t commaPos = lines[i].find(',');
            if (commaPos != std::string::npos){
                statisticsRoundTextbox->label(statisticsRoundTextbox->label() + lines[i].substr(0, commaPos) + "\n"); 
                statisticsOutcomesTextbox->label(statisticsOutcomesTextbox->label() + lines[i].substr(commaPos + 1) + "\n"); 
            }
        }
    }


    //hide all visual components
    void hideAll() {
        playGameButton->hide();
        instructionsButton->hide();
        statsButton->hide();
        statisticsRoundTextbox->hide();
        statisticsOutcomesTextbox->hide();
        quitGameButton->hide();
        instructionsTextBox->hide();
        returnToMainMenuButton->hide();
        playAgainButton->hide();
        giveUpButton->hide();
        gameStateTextbox->hide();
        spinningLetterTextbox->hide();
        for (int i = 0; i < 6; ++i) slotButtons[i]->hide();
    }

    //update game state label
    void updateGameStateLabel() {
        std::string display = "Slots: ";
        for (int i = 0; i < 6; ++i) {
            display += "[ ";
            display += placedLetters[i];
            display += " ]  ";
        }
        gameStateTextbox->label(display);
        gameStateTextbox->align(FL_ALIGN_CENTER);
    }

    //function to handle rendering of the current screen
    void renderCurrentScreen() {
        hideAll();
        if (currentScreen == "Main Menu") {
            titleTextbox->label("ABC Blind Rank");
            playGameButton->show();
            instructionsButton->show();
            statsButton->show();
            quitGameButton->show();
        } else if (currentScreen == "Instructions") {
            titleTextbox->label("How to Play");
            instructionsTextBox->show();
            returnToMainMenuButton->show();
        } else if (currentScreen == "Statistics") {
            readStatsFromFile();
            titleTextbox->label("Statistics");
            statisticsRoundTextbox->show();
            statisticsOutcomesTextbox->show();
            playAgainButton->show();
            returnToMainMenuButton->show();
        } else if (currentScreen == "Game") {
            startNewGame();
        }
    }

    //function to start a new game
    void startNewGame() {
        roundNumber++;

        selectedLetters.clear();
        for (char c = 'A'; c <= 'Z'; ++c) selectedLetters.push_back(c);
        std::random_shuffle(selectedLetters.begin(), selectedLetters.end());
        selectedLetters.resize(6);

        std::fill(std::begin(placedLetters), std::end(placedLetters), '_');
        std::fill(std::begin(slotUsed), std::end(slotUsed), false);

        titleTextbox->label("ABC Blind Rank \n" "Game");

        updateGameStateLabel();
        gameStateTextbox->show();
        spinningLetterTextbox->label("Generating...");
        spinningLetterTextbox->show();
        giveUpButton->show();

        for (int i = 0; i < 6; ++i) {
            slotButtons[i]->activate();
            slotButtons[i]->show();
        }

        currentLetter = selectedLetters.front();
        selectedLetters.erase(selectedLetters.begin());

        isSpinning = true;
        spinCount = 0;
        Fl::add_timeout(0.2, Application::spinStepCallback, this);
    }

     //checks if the letter can be placed properly, if not then the game ends
    bool canPlaceLetter(char letter){
        for(int i=0; i < 6; i++){
            if(slotUsed[i]) continue;

            char left = '\0';
            for(int j = i - 1; j >= 0; j--){
                if(slotUsed[j]){
                    left = placedLetters[j];
                    break;
                }
            }
        
            char right = '\0';
            for(int j = i + 1; j < 6; j++){
                if(slotUsed[j]){
                    right = placedLetters[j];
                    break;
                }
            }

            bool leftOkay = (left == '\0' || left <= letter);
            bool rightOkay = (right == '\0' || letter <= right);

            if(leftOkay && rightOkay) return true;
        }
        return false;
    }

    static void delayedGameOverCallback(void* userdata){
        Application* app  = static_cast<Application*>(userdata);
        app->checkGameOverCannotPlace();
    }

    //checks if letter is placeable
    void checkGameOverCannotPlace(){
        if(!canPlaceLetter(currentLetter)){
            bobcat::showMessage("Letter has no valid placement! Game Over!");
            writeDataToFile(roundNumber, "Loss");
            currentScreen = "Main Menu";
            renderCurrentScreen();
            }
        }


    //spin letters without freezing UI
    void spinStep(){
        if(spinCount < 15){
            char temp = 'A' + rand() % 26;
            spinningLetterTextbox->align(FL_ALIGN_CENTER);
            spinningLetterTextbox->label(std::string("Generating:...") + temp);
            spinCount++;
            Fl::add_timeout(0.2, spinStepCallback, this);
        }else{
            spinningLetterTextbox->label(std::string("Place: ") + currentLetter);
            spinningLetterTextbox->align(FL_ALIGN_CENTER);
            spinningLetterTextbox->labelsize(27);
            isSpinning = false;

            Fl::add_timeout(2.5, Application::delayedGameOverCallback, this);
        }
    }


    //handle navigation buttons
    void handleNavigationClick(bobcat::Widget* sender) {
        if (sender == playGameButton|| sender == playAgainButton) {
            currentScreen = "Game";
            renderCurrentScreen();
        } else if (sender == instructionsButton) {
            currentScreen = "Instructions";
            renderCurrentScreen();
        } else if (sender == statsButton) {
            currentScreen = "Statistics";
            renderCurrentScreen();
        } else if (sender == returnToMainMenuButton) {
            currentScreen = "Main Menu";
            renderCurrentScreen();
        } else if (sender == quitGameButton) {
            window->hide();
        }
    }

    //handle slot clicks
    void handleSlotClick(bobcat::Widget* sender) {
        if (isSpinning) return; // don't allow slot choice while spinning

        for (int i = 0; i < 6; ++i) {
            if (sender == slotButtons[i] && !slotUsed[i]) {
                placedLetters[i] = currentLetter;
                slotUsed[i] = true;
                slotButtons[i]->deactivate();
                updateGameStateLabel();

                if (selectedLetters.empty()) {
                    std::vector<char> finalLetters(placedLetters, placedLetters + 6);
                    if (std::is_sorted(finalLetters.begin(), finalLetters.end())) {
                        bobcat::showMessage("You Win!");
                        writeDataToFile(roundNumber, "Win");
                    } else {
                        bobcat::showMessage("Incorrect Order! You Lose.");
                        writeDataToFile(roundNumber, "Loss");
                    }
                    currentScreen = "Main Menu";
                    renderCurrentScreen();
                    return;
                }

                currentLetter = selectedLetters.front();
                selectedLetters.erase(selectedLetters.begin());

                isSpinning = true;
                spinningLetterTextbox->label("Generating...");
                spinCount = 0;
                Fl::add_timeout(0.2, Application::spinStepCallback, this);
                break;
            }
        }
    }


    //handle give up
    void handleGameClick(bobcat::Widget* sender) {
        if (sender == giveUpButton) {
            bobcat::showMessage("You gave up. Game Over.");
            writeDataToFile(roundNumber, "Loss");
            currentScreen = "Main Menu";
            renderCurrentScreen();
        }
    }

public:
    Application() {
        srand(static_cast<unsigned int>(time(nullptr)));

        window = new bobcat::Window(100, 100, 500, 400, "2025 | CSE 022 | Programming Project");

        currentScreen = "Main Menu";
        loadLastRoundNumber();

        titleTextbox = new bobcat::TextBox(25, 25, 450, 25, "ABC Blind Rank");
        titleTextbox->align(FL_ALIGN_CENTER);
        titleTextbox->labelsize(30);

        // main menu screen
        playGameButton = new bobcat::Button(75, 95, 350, 40, "Play Game");
        instructionsButton = new bobcat::Button(75, 145, 350, 40, "View Instructions");
        statsButton = new bobcat::Button(75, 195, 350, 40, "View Statistics");
        quitGameButton = new bobcat::Button(75, 245, 350, 40, "Quit Game");
        ON_CLICK(playGameButton, Application::handleNavigationClick);
        ON_CLICK(instructionsButton, Application::handleNavigationClick);
        ON_CLICK(statsButton, Application::handleNavigationClick);
        ON_CLICK(quitGameButton, Application::handleNavigationClick);

        // instructions screen
        instructionsTextBox = new bobcat::TextBox(50, 75, 400, 200,
            "- You will be given 6 random letters.\n\n"
            "- Place each letter into one of the 6 slots.\n\n"
            "- You cannot move letters once placed.\n\n"
            "- Try to order them alphabetically!");
        instructionsTextBox->align(FL_ALIGN_CENTER);

        // statistics screen
        statisticsRoundTextbox = new bobcat::TextBox(50, 75, 100, 250, "Round");
        statisticsOutcomesTextbox = new bobcat::TextBox(250, 75, 100, 250, "Outcome");
        statisticsRoundTextbox->align(FL_ALIGN_TOP_LEFT);
        statisticsOutcomesTextbox->align(FL_ALIGN_TOP_LEFT);

        playAgainButton = new bobcat::Button(75, 300, 350, 30, "Play Again");
        ON_CLICK(playAgainButton, Application::handleNavigationClick);
        

        returnToMainMenuButton = new bobcat::Button(75, 350, 350, 30, "Return to Main Menu");
        ON_CLICK(returnToMainMenuButton, Application::handleNavigationClick);

        giveUpButton = new bobcat::Button(75, 350, 350, 30, "Give Up");
        ON_CLICK(giveUpButton, Application::handleGameClick);

        gameStateTextbox = new bobcat::TextBox(25, 200, 450, 25, "Slots: [ _ ]  [ _ ]  [ _ ]  [ _ ]  [ _ ]  [ _ ]");
        gameStateTextbox->labelsize(15);
        gameStateTextbox->align(FL_ALIGN_CENTER);
        spinningLetterTextbox = new bobcat::TextBox(25, 110, 450, 25, "Generating...");

        for (int i = 0; i < 6; ++i) {
            slotButtons[i] = new bobcat::Button(50 + i * 70, 250, 60, 35, "Slot " + std::to_string(i + 1));
            ON_CLICK(slotButtons[i], Application::handleSlotClick);
        }

        renderCurrentScreen();
        window->show();
    }

    friend struct AppTest;
};

#endif
