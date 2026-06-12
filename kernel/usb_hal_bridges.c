#include "pro_os.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_cdc_ecm.h"
#include "usbh_cdc_ncm.h"

/* Sovereign USB Network/Audio Stubs and Bridges */

void usbh_cdc_ecm_run(struct usbh_cdc_ecm *cdc_ecm_class) { (void)cdc_ecm_class; }
void usbh_cdc_ecm_stop(struct usbh_cdc_ecm *cdc_ecm_class) { (void)cdc_ecm_class; }
void usbh_cdc_ecm_eth_input(uint8_t *buf, uint32_t buflen) { (void)buf; (void)buflen; }

void usbh_cdc_ncm_run(struct usbh_cdc_ncm *cdc_ncm_class) { (void)cdc_ncm_class; }
void usbh_cdc_ncm_stop(struct usbh_cdc_ncm *cdc_ncm_class) { (void)cdc_ncm_class; }
void usbh_cdc_ncm_eth_input(uint8_t *buf, uint32_t buflen) { (void)buf; (void)buflen; }
