#include "ImageLib.h"

#include <QResource>
#include <QImage>
#include <QBuffer>
#include <QMap>
#include <opencv2/opencv.hpp>

namespace {// Глобальные переменные, но в пространстве имен DLL
    std::mutex m_mutex;
    QMap<QString, cv::Mat> g_images;
    bool isLoaded = false;
    QString version = "1.0";
    Cleaner clean;
}

Cleaner::~Cleaner(){
    cleanup();
}

extern "C" {

IMAGE_API void loadImages(bool &result) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if(isLoaded) {
        result = isLoaded;
        return;
    }

    g_images.clear();

    QStringList resourcePaths;
    getListImages(resourcePaths);

    for (const QString& resourcePath : resourcePaths) {
        QImage qImage(resourcePath);

        // Конвертируем QImage в cv::Mat
        cv::Mat cvImg;
        switch (qImage.format()) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
            cvImg = cv::Mat(qImage.height(), qImage.width(), CV_8UC4, (void*)qImage.constBits(), qImage.bytesPerLine());
            cvtColor(cvImg, cvImg, cv::COLOR_RGBA2RGB);
            break;
        case QImage::Format_Grayscale8:
            cvImg = cv::Mat(qImage.height(), qImage.width(), CV_8UC1, (void*)qImage.bits(), qImage.bytesPerLine());
            break;
        default:
            break;
        }
        g_images[resourcePath] = cvImg.clone();
    }

    result = !g_images.isEmpty();
    isLoaded = result;
}

IMAGE_API void getImage(const QString &path, cv::Mat &image) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (g_images.contains(path)) {
        image.release();
        image = g_images[path].clone();
    }
}

IMAGE_API void hasImage(const QString &path, bool &result) {
    std::lock_guard<std::mutex> lock(m_mutex);
    result = g_images.contains(path);
}

IMAGE_API void updateImage(const QString &path, const cv::Mat *image) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (image) g_images[path] = image->clone();
}

IMAGE_API void cleanup() {
    g_images.clear();
    isLoaded = false;
    version.clear();
}

IMAGE_API void getVersion(QString &result) {
    std::lock_guard<std::mutex> lock(m_mutex);
    result = version;
}

IMAGE_API void getListImages(QStringList &result) {
    QStringList resourcePaths = QStringList()
    << ":/numbers/0.png"
    << ":/numbers/1.png"
    << ":/numbers/2.png"
    << ":/numbers/3.png"
    << ":/numbers/4.png"
    << ":/numbers/5.png"
    << ":/numbers/6.png"
    << ":/numbers/7.png"
    << ":/numbers/8.png"
    << ":/numbers/9.png"
    << ":/pages/arena/battles/button_1.png"
    << ":/pages/arena/battles/button_10.png"
    << ":/pages/arena/battles/button_2.png"
    << ":/pages/arena/battles/button_3.png"
    << ":/pages/arena/battles/button_4.png"
    << ":/pages/arena/battles/button_5.png"
    << ":/pages/arena/battles/button_6.png"
    << ":/pages/arena/battles/button_7.png"
    << ":/pages/arena/battles/button_8.png"
    << ":/pages/arena/battles/button_9.png"
    << ":/pages/arena/battles/button_home.png"
    << ":/pages/arena/battles/compare.png"
    << ":/pages/arena/battles/compare_me.png"
    << ":/pages/arena/battles/sample.png"
    << ":/pages/arena/battles/sample_end.png"
    << ":/pages/arena/battles/sample_me.png"
    << ":/pages/arena/battles/sample_next.png"
    << ":/pages/arena/battles/sample_select.png"
    << ":/pages/arena/battles/state_stage.png"
    << ":/pages/arena/battles/state_wait.png"
    << ":/pages/arena/find/button_cancel.png"
    << ":/pages/arena/find/button_close.png"
    << ":/pages/arena/find/button_no.png"
    << ":/pages/arena/find/button_yes.png"
    << ":/pages/arena/find/compare.png"
    << ":/pages/arena/find/compare_cancel.png"
    << ":/pages/arena/find/sample.png"
    << ":/pages/arena/find/sample_cancel.png"
    << ":/pages/arena/main/button_apples.png"
    << ":/pages/arena/main/button_close.png"
    << ":/pages/arena/main/button_start.png"
    << ":/pages/arena/main/button_ticket.png"
    << ":/pages/arena/main/compare.png"
    << ":/pages/arena/main/sample.png"
    << ":/pages/arena/main/sample_apples.png"
    << ":/pages/arena/main/sample_ticket.png"
    << ":/pages/arena/main/state_apples.png"
    << ":/pages/arena/main/state_ticket.png"
    << ":/pages/battle/arena/button_close.png"
    << ":/pages/battle/arena/button_next.png"
    << ":/pages/battle/arena/button_qstart.png"
    << ":/pages/battle/arena/button_start.png"
    << ":/pages/battle/arena/compare.png"
    << ":/pages/battle/arena/enemy_name.png"
    << ":/pages/battle/arena/enemy_power.png"
    << ":/pages/battle/arena/my_power.png"
    << ":/pages/battle/arena/sample.png"
    << ":/pages/battle/arena/sample_available.png"
    << ":/pages/battle/arena/sample_defeat.png"
    << ":/pages/battle/arena/sample_victory.png"
    << ":/pages/battle/arena/state_attack.png"
    << ":/pages/battle/arena/state_battle.png"
    << ":/pages/battle/dark/button_best.png"
    << ":/pages/battle/dark/button_qstart.png"
    << ":/pages/battle/dark/button_start.png"
    << ":/pages/battle/dark/compare.png"
    << ":/pages/battle/dark/no_unit.png"
    << ":/pages/battle/dark/sample.png"
    << ":/pages/battle/dark/state_power.png"
    << ":/pages/battle/end/button_ads.png"
    << ":/pages/battle/end/button_home.png"
    << ":/pages/battle/end/compare.png"
    << ":/pages/battle/end/sample.png"
    << ":/pages/battle/end/sample_defeat.png"
    << ":/pages/battle/end/sample_show_ads.png"
    << ":/pages/battle/end/sample_victory.png"
    << ":/pages/battle/end/state_ads.png"
    << ":/pages/battle/end/state_victory.png"
    << ":/pages/battle/general/button_speed.png"
    << ":/pages/battle/general/compare_damage.png"
    << ":/pages/battle/general/compare_effects.png"
    << ":/pages/battle/general/compare_hero.png"
    << ":/pages/battle/general/compare_speed.png"
    << ":/pages/battle/general/sample.png"
    << ":/pages/battle/lighthouse/button_qstart.png"
    << ":/pages/battle/lighthouse/button_start.png"
    << ":/pages/battle/lighthouse/compare.png"
    << ":/pages/battle/lighthouse/compare_no_unit.png"
    << ":/pages/battle/lighthouse/sample.png"
    << ":/pages/battle/lighthouse/sample_no_unit.png"
    << ":/pages/dark/button_ads.png"
    << ":/pages/dark/button_apples.png"
    << ":/pages/dark/button_close.png"
    << ":/pages/dark/button_end.png"
    << ":/pages/dark/button_keys.png"
    << ":/pages/dark/button_shop.png"
    << ":/pages/dark/button_start.png"
    << ":/pages/dark/compare.png"
    << ":/pages/dark/compare_end.png"
    << ":/pages/dark/sample.png"
    << ":/pages/dark/sample_ads.png"
    << ":/pages/dark/sample_ads_unavailable.png"
    << ":/pages/dark/sample_end.png"
    << ":/pages/dark/sample_hard.png"
    << ":/pages/dark/sample_insane.png"
    << ":/pages/dark/sample_normal.png"
    << ":/pages/dark/state_ads.png"
    << ":/pages/dark/state_apples.png"
    << ":/pages/dark/state_end.png"
    << ":/pages/dark/state_hard.png"
    << ":/pages/dark/state_insane.png"
    << ":/pages/dark/state_keys.png"
    << ":/pages/dark/state_normal.png"
    << ":/pages/dark/map/find_0.png"
    << ":/pages/dark/map/find_1.png"
    << ":/pages/dark/map/find_2.png"
    << ":/pages/dark/map/find_3.png"
    << ":/pages/dark/map/find_4.png"
    << ":/pages/dark/map/find_5.png"
    << ":/pages/dark/map/find_6.png"
    << ":/pages/dark/map/sample_0.png"
    << ":/pages/dark/map/sample_1.png"
    << ":/pages/dark/map/sample_2.png"
    << ":/pages/dark/map/sample_3.png"
    << ":/pages/dark/map/sample_4.png"
    << ":/pages/dark/map/sample_5.png"
    << ":/pages/dark/map/sample_6.png"
    << ":/pages/dark/waypoints/button_close.png"
    << ":/pages/dark/waypoints/button_end.png"
    << ":/pages/dark/waypoints/button_next.png"
    << ":/pages/dark/waypoints/compare_complete.png"
    << ":/pages/dark/waypoints/compare_stage.png"
    << ":/pages/dark/waypoints/sample_complete.png"
    << ":/pages/dark/waypoints/sample_end.png"
    << ":/pages/dark/waypoints/sample_next.png"
    << ":/pages/dark/waypoints/stage_1.png"
    << ":/pages/dark/waypoints/stage_2.png"
    << ":/pages/dark/waypoints/stage_3.png"
    << ":/pages/dark/waypoints/state_end.png"
    << ":/pages/dark/waypoints/state_next.png"
    << ":/pages/dark/waypoints/altar/button_confirm.png"
    << ":/pages/dark/waypoints/altar/button_move.png"
    << ":/pages/dark/waypoints/altar/compare.png"
    << ":/pages/dark/waypoints/altar/sample.png"
    << ":/pages/dark/waypoints/blessing/button_1.png"
    << ":/pages/dark/waypoints/blessing/button_2.png"
    << ":/pages/dark/waypoints/blessing/button_3.png"
    << ":/pages/dark/waypoints/blessing/button_confirm.png"
    << ":/pages/dark/waypoints/blessing/compare.png"
    << ":/pages/dark/waypoints/blessing/compareS.png"
    << ":/pages/dark/waypoints/blessing/sample.png"
    << ":/pages/dark/waypoints/camp/button_confirm.png"
    << ":/pages/dark/waypoints/camp/button_heal.png"
    << ":/pages/dark/waypoints/camp/button_res.png"
    << ":/pages/dark/waypoints/camp/button_reward.png"
    << ":/pages/dark/waypoints/camp/compare.png"
    << ":/pages/dark/waypoints/camp/sample.png"
    << ":/pages/dark/waypoints/game/button_confirm.png"
    << ":/pages/dark/waypoints/game/button_risk.png"
    << ":/pages/dark/waypoints/game/button_skip.png"
    << ":/pages/dark/waypoints/game/coin_lose.png"
    << ":/pages/dark/waypoints/game/coin_mask.png"
    << ":/pages/dark/waypoints/game/coin_win.png"
    << ":/pages/dark/waypoints/game/compare.png"
    << ":/pages/dark/waypoints/game/sample.png"
    << ":/pages/dark/waypoints/reflection/button_confirm.png"
    << ":/pages/dark/waypoints/reflection/compare.png"
    << ":/pages/dark/waypoints/reflection/sample.png"
    << ":/pages/event/compare.png"
    << ":/pages/event/sample.png"
    << ":/pages/lighthouse/button_1.png"
    << ":/pages/lighthouse/button_2.png"
    << ":/pages/lighthouse/button_3.png"
    << ":/pages/lighthouse/button_4.png"
    << ":/pages/lighthouse/button_5.png"
    << ":/pages/lighthouse/button_energy.png"
    << ":/pages/lighthouse/button_refresh.png"
    << ":/pages/lighthouse/compare.png"
    << ":/pages/lighthouse/compare_energy.png"
    << ":/pages/lighthouse/compare_refresh.png"
    << ":/pages/lighthouse/enemy_1.png"
    << ":/pages/lighthouse/enemy_2.png"
    << ":/pages/lighthouse/enemy_3.png"
    << ":/pages/lighthouse/enemy_4.png"
    << ":/pages/lighthouse/enemy_5.png"
    << ":/pages/lighthouse/no_energy.png"
    << ":/pages/lighthouse/power_1.png"
    << ":/pages/lighthouse/power_2.png"
    << ":/pages/lighthouse/power_3.png"
    << ":/pages/lighthouse/power_4.png"
    << ":/pages/lighthouse/power_5.png"
    << ":/pages/lighthouse/sample.png"
    << ":/pages/lighthouse/sample_diamond_refresh.png"
    << ":/pages/lighthouse/sample_free_refresh.png"
    << ":/pages/load/button_close_mail.png"
    << ":/pages/load/button_offer_close.png"
    << ":/pages/load/compare.png"
    << ":/pages/load/compare_logo.png"
    << ":/pages/load/compare_mail.png"
    << ":/pages/load/compare_offer.png"
    << ":/pages/load/compare_open.png"
    << ":/pages/load/compare_pass.png"
    << ":/pages/load/sample.png"
    << ":/pages/load/sample_logo.png"
    << ":/pages/load/sample_mail.png"
    << ":/pages/load/sample_offer.png"
    << ":/pages/load/sample_open.png"
    << ":/pages/load/sample_pass.png"
    << ":/pages/main/button_apple.png"
    << ":/pages/main/button_book.png"
    << ":/pages/main/button_chat.png"
    << ":/pages/main/button_ct.png"
    << ":/pages/main/button_cw.png"
    << ":/pages/main/button_friends.png"
    << ":/pages/main/button_gold.png"
    << ":/pages/main/button_iron.png"
    << ":/pages/main/button_mail.png"
    << ":/pages/main/button_mana.png"
    << ":/pages/main/button_map.png"
    << ":/pages/main/button_mithril.png"
    << ":/pages/main/button_settings.png"
    << ":/pages/main/button_shop.png"
    << ":/pages/main/button_user.png"
    << ":/pages/main/button_wood.png"
    << ":/pages/main/compare.png"
    << ":/pages/main/compare_1.png"
    << ":/pages/main/sample.png"
    << ":/pages/main/sample_cw.png"
    << ":/pages/main/state_cw.png"
    << ":/pages/map/button_arena.png"
    << ":/pages/map/button_close.png"
    << ":/pages/map/button_dark.png"
    << ":/pages/map/button_dung.png"
    << ":/pages/map/button_gem.png"
    << ":/pages/map/button_invasion.png"
    << ":/pages/map/button_journey.png"
    << ":/pages/map/button_lighthouse.png"
    << ":/pages/map/button_portal.png"
    << ":/pages/map/button_submap.png"
    << ":/pages/map/button_tower.png"
    << ":/pages/map/compare.png"
    << ":/pages/map/compare_left.png"
    << ":/pages/map/compare_right.png"
    << ":/pages/map/sample.png"
    << ":/pages/map/sample_invasion.png"
    << ":/pages/map/sample_right.png"
    << ":/pages/map/state_invasion.png"
    << ":/pages/settings/button_en.png"
    << ":/pages/settings/button_fps.png"
    << ":/pages/settings/button_lang.png"
    << ":/pages/settings/button_no.png"
    << ":/pages/settings/button_yes.png"
    << ":/pages/settings/compare.png"
    << ":/pages/settings/compare_change_lang.png"
    << ":/pages/settings/compare_confirm.png"
    << ":/pages/settings/sample.png"
    << ":/pages/settings/sample_change_lang.png"
    << ":/pages/settings/sample_confirm.png"
    << ":/pages/settings/state_fps.png"
    << ":/pages/settings/state_lang.png"
    << ":/pages/squad/arena/button_best.png"
    << ":/pages/squad/arena/button_close.png"
    << ":/pages/squad/arena/button_previous.png"
    << ":/pages/squad/arena/button_start.png"
    << ":/pages/squad/arena/compare.png"
    << ":/pages/squad/arena/power.png"
    << ":/pages/squad/arena/sample.png"
    << ":/pages/squad/dark/button_best.png"
    << ":/pages/squad/dark/button_previous.png"
    << ":/pages/squad/dark/button_start.png"
    << ":/pages/squad/dark/compare.png"
    << ":/pages/squad/dark/sample.png"
    << ":/pages/squad/dark/state_hero.png"
    << ":/pages/squad/dark/state_power.png"
    << ":/pages/squad/main/button_info.png"
    << ":/pages/squad/main/button_pals.png"
    << ":/pages/squad/main/compare.png"
    << ":/pages/squad/main/compare_barrack.png"
    << ":/pages/squad/main/compare_info.png"
    << ":/pages/squad/main/compare_pals.png"
    << ":/pages/squad/main/count.png"
    << ":/pages/squad/main/power.png"
    << ":/pages/squad/main/sample.png"
    << ":/pages/squad/main/sample_barrack.png"
    << ":/pages/squad/main/sample_info.png"
    << ":/pages/squad/main/unit_1.png"
    << ":/pages/squad/main/unit_2.png"
    << ":/pages/squad/main/unit_3.png"
    << ":/pages/squad/main/unit_4.png"
    << ":/pages/squad/main/unit_5.png"
    << ":/pages/squad/main/unit_6.png"
    << ":/pages/squad/main/unit_7.png"
    << ":/pages/squad/main/unit_8.png"
    << ":/pages/squad/main/unit/button_confirm.png"
    << ":/pages/squad/main/unit/button_no.png"
    << ":/pages/squad/main/unit/button_set.png"
    << ":/pages/squad/main/unit/button_set_1.png"
    << ":/pages/squad/main/unit/button_set_2.png"
    << ":/pages/squad/main/unit/button_set_3.png"
    << ":/pages/squad/main/unit/button_yes.png"
    << ":/pages/squad/main/unit/compare.png"
    << ":/pages/squad/main/unit/compare_confirm.png"
    << ":/pages/squad/main/unit/compare_set.png"
    << ":/pages/squad/main/unit/confirm_warning.png"
    << ":/pages/squad/main/unit/sample.png"
    << ":/pages/squad/main/unit/sample_confirm.png"
    << ":/pages/squad/main/unit/sample_set.png"
    << ":/pages/squad/main/unit/sample_set_0.png"
    << ":/pages/squad/main/unit/sample_set_1.png"
    << ":/pages/squad/main/unit/sample_set_2.png"
    << ":/pages/squad/main/unit/sample_set_3.png"
    << ":/pages/squad/main/unit/state_set.png"
    << ":/pages/squad/main/unit/state_unequip.png"
    << ":/pages/top_players/button_close.png"
    << ":/pages/top_players/button_friends.png"
    << ":/pages/top_players/button_visit.png"
    << ":/pages/top_players/compare.png"
    << ":/pages/top_players/compare_1.png"
    << ":/pages/top_players/compare_top.png"
    << ":/pages/top_players/compare_visit.png"
    << ":/pages/top_players/sample.png"
    << ":/pages/top_players/sample_top.png"
    << ":/pages/top_players/sample_visit.png"
    << ":/pages/user/button_battles.png"
    << ":/pages/user/button_close.png"
    << ":/pages/user/button_hero.png"
    << ":/pages/user/compare.png"
    << ":/pages/user/compare_battles.png"
    << ":/pages/user/sample.png"
    << ":/pages/user/sample_battles.png"
    << ":/pages/user/state_ads.png"
    << ":/pages/user/state_prem.png"
    << ":/pages/user/user_id.png"
    << ":/pages/user/user_power.png"
    << ":/pages/warnings/dark/button_no.png"
    << ":/pages/warnings/dark/button_yes.png"
    << ":/pages/warnings/dark/compare.png"
    << ":/pages/warnings/dark/sample_1.png"
    << ":/pages/warnings/dark/sample_2.png"
    << ":/pages/warnings/dark/sample_floor.png"
    << ":/pages/warnings/dark/sample_point.png"
    << ":/pages/warnings/dark/sample_select.png"
    << ":/pages/warnings/dark/sample_spells.png"
    << ":/pages/warnings/dark/sample_units.png"
    << ":/pages/warnings/general/button_daily.png"
    << ":/pages/warnings/general/button_device.png"
    << ":/pages/warnings/general/button_sleep.png"
    << ":/pages/warnings/general/compare_daily.png"
    << ":/pages/warnings/general/compare_device.png"
    << ":/pages/warnings/general/compare_goblin.png"
    << ":/pages/warnings/general/compare_sleep.png"
    << ":/pages/warnings/general/sample_daily.png"
    << ":/pages/warnings/general/sample_device.png"
    << ":/pages/warnings/general/sample_goblin.png"
    << ":/pages/warnings/general/sample_sleep.png"
    << ":/pages/warnings/lighthouse/button_no.png"
    << ":/pages/warnings/lighthouse/button_yes.png"
    << ":/pages/warnings/lighthouse/compare.png"
    << ":/pages/warnings/lighthouse/sample.png"
        ;
    result.clear();
    result = resourcePaths;
}

} // extern "C"
