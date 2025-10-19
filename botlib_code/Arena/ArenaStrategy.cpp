#include "Arena/ArenaStrategy.h"
#include "Arena/ArenaPlayer.h"
#include <QList>

bool ArenaStrategy::check(const ArenaPlayer &temp, int maxPower){
    if(temp.status != State::AVAILABLE) return false;
    if(temp.blackList && black) return false;
    if(temp.whiteList && white) return true;
    if((temp.power > maxPower)) return false;
    return false;
}

int ArenaStrategy::getPosition(const QList<ArenaPlayer> &listPlayers, int stage, int maxPower){
    switch (stage){
    case 1:{
        if(first){
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(!check(listPlayers[i],maxPower)) continue;
                return i+1;
            }
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(listPlayers[i].status != State::AVAILABLE) continue;
                return i+1;
            }
        }
        else {
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(!check(listPlayers[i-1],maxPower)) continue;
                return i;
            }
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(listPlayers[i-1].status != State::AVAILABLE) continue;
                return i;
            }
        }
        break;
    }
    case 2:{
        if(second){
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(!check(listPlayers[i],maxPower)) continue;
                return i+1;
            }
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(listPlayers[i].status != State::AVAILABLE) continue;
                return i+1;
            }
        }
        else {
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(!check(listPlayers[i-1],maxPower)) continue;
                return i;
            }
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(listPlayers[i-1].status != State::AVAILABLE) continue;
                return i;
            }
        }
        break;
    }
    case 3:{
        if(third){
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(!check(listPlayers[i],maxPower)) continue;
                return i+1;
            }
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(listPlayers[i].status != State::AVAILABLE) continue;
                return i+1;
            }
        }
        else {
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(!check(listPlayers[i-1],maxPower)) continue;
                return i;
            }
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(listPlayers[i-1].status != State::AVAILABLE) continue;
                return i;
            }
        }
        break;
    }
    case 4:{
        if(fourth){
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(!check(listPlayers[i],maxPower)) continue;
                return i+1;
            }
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(listPlayers[i].status != State::AVAILABLE) continue;
                return i+1;
            }
        }
        else {
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(!check(listPlayers[i-1],maxPower)) continue;
                return i;
            }
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(listPlayers[i-1].status != State::AVAILABLE) continue;
                return i;
            }
        }
        break;
    }
    case 5:{
        if(fives){
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(!check(listPlayers[i],maxPower)) continue;
                return i+1;
            }
            for(int i = 0,n = listPlayers.count();i<n;i++){
                if(listPlayers[i].status != State::AVAILABLE) continue;
                return i+1;
            }
        }
        else {
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(!check(listPlayers[i-1],maxPower)) continue;
                return i;
            }
            for(int n = 0,i = listPlayers.count();i>n;i--){
                if(listPlayers[i-1].status != State::AVAILABLE) continue;
                return i;
            }
        }
        break;
    }
    default:   ;
    }
    return -1;
}
