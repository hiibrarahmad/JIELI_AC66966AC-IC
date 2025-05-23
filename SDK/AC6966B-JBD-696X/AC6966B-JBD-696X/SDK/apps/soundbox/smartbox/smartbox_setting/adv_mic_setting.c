#include "app_config.h"
#include "syscfg_id.h"
#include "le_smartbox_module.h"

#include "adv_mic_setting.h"
#include "smartbox_adv_common.h"
#include "smartbox_setting_sync.h"
#include "smartbox_setting_opt.h"

//#if (SMART_BOX_EN && RCSP_SMARTBOX_ADV_EN)
#if (SMART_BOX_EN)
#if RCSP_ADV_MIC_SET_ENABLE

extern void test_esco_role_switch(u8 flag);
extern void tws_conn_switch_role();
extern int get_bt_tws_connect_status();
static void mic_setting_info_deal(u8 *mic_info_data)
{
#if TCFG_USER_TWS_ENABLE
    u8 channel = tws_api_get_local_channel();
    printf("%s, %d\n", __FUNCTION__, channel);

    if (tws_api_get_role() == TWS_ROLE_MASTER) {
        printf("-----master-----\n");
    } else {
        printf("-----role-----\n");
    }
    switch (*mic_info_data) {
    case  0x01:
        if (get_bt_tws_connect_status()) {
            printf("-------auto--------\n");
            tws_api_auto_role_switch_enable();
        }
        break;
    case  0x02:
        if (get_bt_tws_connect_status()) {
            if (channel != 'L') {
                printf("-------!L--------\n");
                tws_api_auto_role_switch_disable();
                tws_conn_switch_role();
            }
        }
        break;
    case  0x03:
        if (get_bt_tws_connect_status()) {
            if (channel != 'R') {
                printf("-------!R--------\n");
                tws_api_auto_role_switch_disable();
                tws_conn_switch_role();
            }
        }
        break;
    default:
        break;
    }
#endif
}

static void set_mic_setting(u8 *mic_setting_info)
{
    _s_info.mic_mode = 	*mic_setting_info;
}

static void get_mic_setting(u8 *mic_setting_info)
{
    *mic_setting_info = _s_info.mic_mode;
}

static void update_mic_setting_vm_value(u8 *mic_setting_info)
{
    syscfg_write(CFG_RCSP_ADV_MIC_SETTING, mic_setting_info, 1);
}

static void adv_mic_setting_sync(u8 *mic_setting_info)
{
#if TCFG_USER_TWS_ENABLE
    if (get_bt_tws_connect_status()) {
        update_smartbox_setting(BIT(ATTR_TYPE_MIC_SETTING));
    }
#endif
}

static void deal_mic_setting(u8 *mic_setting_info, u8 write_vm, u8 tws_sync)
{
    if (mic_setting_info) {
        set_mic_setting(mic_setting_info);
    }
    if (write_vm) {
        update_mic_setting_vm_value(&_s_info.mic_mode);
    }
    if (tws_sync) {
        adv_mic_setting_sync(&_s_info.mic_mode);
    }
    mic_setting_info_deal(&_s_info.mic_mode);
}


//通话时,固定mic的位置,mode--esco state
void rcsp_user_mic_fixed_deal(u8 mode)
{
    u8 channel = tws_api_get_local_channel();

    if (!get_bt_tws_connect_status()) {
        return;
    }

    if (mode == 0) {
        test_esco_role_switch(0);
        return;
    }

    u8 adv_mic_setting = 0;
    get_mic_setting(&adv_mic_setting);
    if ((tws_api_get_role() == TWS_ROLE_MASTER)
        && (tws_api_get_tws_state() | TWS_STA_ESCO_OPEN)) {

        switch (adv_mic_setting) {
        case 0x02:
            if (get_bt_tws_connect_status()) {
                if (channel != 'L') {
                    printf("mic_sw_l\n");
                    test_esco_role_switch(1);
                }
            }
            break;

        case 0x03:
            if (get_bt_tws_connect_status()) {
                if (channel != 'R') {
                    printf("mic_sw_r\n");
                    test_esco_role_switch(1);

                }
            }

            break;

        default:
            printf("mic_sw_auto\n");
            test_esco_role_switch(0);
            break;
        }
    }
}

static SMARTBOX_SETTING_OPT adv_mic_opt = {
    .data_len = 1,
    .setting_type =	ATTR_TYPE_MIC_SETTING,
    .syscfg_id = CFG_RCSP_ADV_MIC_SETTING,
    .deal_opt_setting = deal_mic_setting,
    .set_setting = set_mic_setting,
    .get_setting = get_mic_setting,
    .custom_setting_init = NULL,
    .custom_vm_info_update = NULL,
    .custom_setting_update = NULL,
    .custom_sibling_setting_deal = NULL,
    .custom_setting_release = NULL,
};
REGISTER_APP_SETTING_OPT(adv_mic_opt);

#endif

#endif
