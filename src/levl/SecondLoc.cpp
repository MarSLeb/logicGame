#include "SecondLoc.h"

SecondLoc::~SecondLoc() {}

SecondLoc::SecondLoc(int save, shared_ptr<RenderWindow> window, shared_ptr<Player> player,
shared_ptr<Book> book, bool havingBook, int listBool):
save(save), window(window), player(player), book(book), havingBook(havingBook){
    hasNote = listBool == 1;
    
    for(int i = 0; i < 3; i++) {potionTexture[i].loadFromFile("foto/potion/" + to_string(i + 1) + ".png");}

    array<int, 7> textureIds {0, 2, 2, 1, 1, 0, 2};
    for(int i = 0; i < 7; i++) {
        potion[i].setTexture(potionTexture[textureIds[i]]);
    }
    
    for(int i = 0, y = 320; i < 7; i++, y+= 15) {potion[i].setPosition(30, y);}
    curPotion = 8;
    finishPotion = false;

    texture.loadFromFile("foto/second.png");
    listTexture.loadFromFile("foto/potion/list.png");
    havingBook == true ? keyTexture.loadFromFile("foto/key.png") :
                         keyTexture.loadFromFile("foto/keyNoBook.png");
    upTexture.loadFromFile("foto/secondUp.png");
    list.setTexture(&listTexture);
    noticeTexture.loadFromFile("foto/notion.png");
    notice.setTexture(&noticeTexture);

    RectangleShape board;
    board = RectangleShape(Vector2f(260, 230)); // bookcase
    board.setPosition(0, 0);
    boards.push_back(board);
    board = RectangleShape(Vector2f(375, 185)); // door
    board.setPosition(260, 0);
    boards.push_back(board);
    board = RectangleShape(Vector2f(350, 245)); // sofa
    board.setPosition(635, 0);
    boards.push_back(board);
    board = RectangleShape(Vector2f(80, 90)); // table
    board.setPosition(0, 345);
    boards.push_back(board);
    board = RectangleShape(Vector2f(960, 250)); // down bord
    board.setPosition(0, 505);
    boards.push_back(board);

    potionPuzzle = Potion(window, book, havingBook ? Pickup::Book : Pickup::None);
    setting = Setting(save, window);
    deed = make_unique<Deed>(window);

    bufferForNotion.loadFromFile("muziek/notion.ogg");
    notionSound.setBuffer(bufferForNotion);
    notionSound.setVolume(40);
    music.openFromFile("muziek/2.ogg");
    music.setVolume(20);
    music.setLoop(true);
}

void SecondLoc::setCurPotion(int num) {curPotion = num; finishPotion = (num != 8);}

string SecondLoc::createSaveString(){
    return ("2" + to_string(book->getPageTwo()) + " " +
            to_string(hasNote == true) + to_string(curPotion) + to_string(havingBook));
}

void SecondLoc::drawList(){
    if(hasNote) {return;}
    window->draw(list);
}

void SecondLoc::died(){
    setting.died();
    deed->died(soundIsPlay);
}

void SecondLoc::toggleSound(bool flag) {soundIsPlay = flag;}

void SecondLoc::drawAll(float time){
    window->clear();
    window->draw(background);
    drawList();
    player->draw();
    window->draw(upground); 
    drawPotion();
    window->draw(key);
    if(hasNote && !havingBook) { window->draw(listKey); }
    if(!noticeEnd && time < 2.5) { window->draw(notice); }
    if(noticeIsRun && time >= 2.5) { noticeEnd = true; }
    window->display();
}

void SecondLoc::drawPotion(){
    for(int i = 0; i < 7; i++){
        if(i != curPotion) {window->draw(potion[i]);}
    }
}

void SecondLoc::addNote(){
    if(!havingBook && hasNote){
        listKeyTexture.loadFromFile("foto/potion/listKey.png");
        listKey.setTexture(&listKeyTexture);
        noteTexture.loadFromFile("foto/potion/listLikeBook.png");
        note.setTexture(&noteTexture);
    }
}

void SecondLoc::drawNote(){
    window->draw(listKey);
    window->draw(note);
    window->display();

    while (window->isOpen()){
        Event ev;
        while(window->pollEvent(ev)){
            if(ev.type == Event::KeyReleased && ev.key.code == Keyboard::F){
                return;
            }
        }
    }
   
}

bool SecondLoc::runSetting(){
    drawAll(0);
    bool result = setting.result(createSaveString(), soundIsPlay);
    music.getStatus() == SoundSource::Status::Paused ? music.play() : music.pause();
    return result;
}

void SecondLoc::run(){
    background.setTexture(&texture);
    upground.setTexture(&upTexture);
    key.setTexture(&keyTexture);
    player->setPosition(535, 405);
    addNote();

    if(soundIsPlay) {music.play();}

    drawAll(0);

    Clock clock;
    Clock clockForNotice;
    while(window->isOpen()){
        Event ev;
        if(!noticeIsRun) {clockForNotice.restart();}
        while(window->pollEvent(ev)){
            if(ev.type == Event::LostFocus) { if(!runSetting()) { return; } }
            if(ev.type == Event::KeyReleased){
                if(ev.key.code == Keyboard::Escape) { if(!runSetting()) { return; } }
                if(ev.key.code == Keyboard::F){
                    drawAll(clockForNotice.getElapsedTime().asSeconds());
                    if(havingBook){
                        book->run();
                    }
                    else if(hasNote){
                        drawNote();
                    }
                    
                }
                if(ev.key.code == Keyboard::T){
                    if(!noticeIsRun && player->getX() >= 800 && player->getX() <= 960 && player->getY() >= 240){
                        hasNote = true;
                        book->setPage(NoteSecond::all); 
                        addNote();
                        noticeIsRun = true; 
                        noticeEnd = false;
                        if(soundIsPlay) {notionSound.play();}
                    }
                    if(player->getY() >= 250 && player->getX() <= 90 && !finishPotion){
                        curPotion = potionPuzzle.run(hasNote);
                        if(curPotion != 8) {finishPotion = true;}
                        if(curPotion == 6 || curPotion == 3 || curPotion == 2) {music.pause(); died(); return;}
                    }
                    if(player->getX() >= 330 && player->getX() <= 440 && player->getY() <= 195){
                        Street loc = Street(save, window, book, player, havingBook);
                        music.pause(); 
                        loc.toggleSound(soundIsPlay); 
                        loc.run(curPotion); 
                        return;
                    }
                }
            }
        }
        
        float time = clock.getElapsedTime().asMicroseconds(); 
		clock.restart(); 
	    time = time / 800; 
        player->movePlayer(time, soundIsPlay, boards);
        drawAll(clockForNotice.getElapsedTime().asSeconds());   
    }
}