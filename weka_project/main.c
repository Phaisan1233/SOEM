#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>
#include "ethercat.h"

#include<stdlib.h>

#include <time.h>

#define EC_TIMEOUTMON 500


char IOmap[128];
double usedmem;
int expectedWKC;
volatile int wkc;
char hstr[1024];
int os;
int16 ob2;
boolean inOP;
boolean needlf;
uint8 currentgroup = 0;
OSAL_THREAD_HANDLE thread1;

int EL7031setup(uint16 slave) {
    int retval;
    uint16 u16val;
    retval = 0;
    /* set some motor parameters, just as example */
    u16val = 1; // max motor current in mA
    retval += ec_SDOwrite(slave, 0x7010, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTSAFE);

    /* set other necessary parameters as needed */
    printf("EL7031 slave %d set, retval = %d\n", slave, retval);
    return 1;
}

char *dtype2string(uint16 dtype) {
    switch (dtype) {
        case ECT_BOOLEAN:
            sprintf(hstr, "BOOLEAN");
            break;
        case ECT_INTEGER8:
            sprintf(hstr, "INTEGER8");
            break;
        case ECT_INTEGER16:
            sprintf(hstr, "INTEGER16");
            break;
        case ECT_INTEGER32:
            sprintf(hstr, "INTEGER32");
            break;
        case ECT_INTEGER24:
            sprintf(hstr, "INTEGER24");
            break;
        case ECT_INTEGER64:
            sprintf(hstr, "INTEGER64");
            break;
        case ECT_UNSIGNED8:
            sprintf(hstr, "UNSIGNED8");
            break;
        case ECT_UNSIGNED16:
            sprintf(hstr, "UNSIGNED16");
            break;
        case ECT_UNSIGNED32:
            sprintf(hstr, "UNSIGNED32");
            break;
        case ECT_UNSIGNED24:
            sprintf(hstr, "UNSIGNED24");
            break;
        case ECT_UNSIGNED64:
            sprintf(hstr, "UNSIGNED64");
            break;
        case ECT_REAL32:
            sprintf(hstr, "REAL32");
            break;
        case ECT_REAL64:
            sprintf(hstr, "REAL64");
            break;
        case ECT_BIT1:
            sprintf(hstr, "BIT1");
            break;
        case ECT_BIT2:
            sprintf(hstr, "BIT2");
            break;
        case ECT_BIT3:
            sprintf(hstr, "BIT3");
            break;
        case ECT_BIT4:
            sprintf(hstr, "BIT4");
            break;
        case ECT_BIT5:
            sprintf(hstr, "BIT5");
            break;
        case ECT_BIT6:
            sprintf(hstr, "BIT6");
            break;
        case ECT_BIT7:
            sprintf(hstr, "BIT7");
            break;
        case ECT_BIT8:
            sprintf(hstr, "BIT8");
            break;
        case ECT_VISIBLE_STRING:
            sprintf(hstr, "VISIBLE_STRING");
            break;
        case ECT_OCTET_STRING:
            sprintf(hstr, "OCTET_STRING");
            break;
        default:
            sprintf(hstr, "Type 0x%4.4X", dtype);
    }
    return hstr;
}

//uint8 get_output_bit (uint16 slave_no,uint8 module_index)
//{
//    /* Get the the startbit position in slaves IO byte */
//    uint8 startbit = ec_slave[slave_no].Ostartbit;
//    /* Mask bit and return boolean 0 or 1 */
//    if (*ec_slave[slave_no].outputs & BIT (module_index - 1  + startbit))
//        return 1;
//    else
//        return 0;
//}

void set_output_bit(uint16 slave_no, uint8 module_index, uint8 value) {
    /* Get the the startbit position in slaves IO byte */
    uint8 startbit = ec_slave[slave_no].Ostartbit;
    /* Set or Clear bit */
    if (value == 0)
        *ec_slave[slave_no].outputs &= ~(1 << (module_index - 1 + startbit));
    else
        *ec_slave[slave_no].outputs |= (1 << (module_index - 1 + startbit));
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void setup() {
    char ifname[] = "enp2s0";
    needlf = FALSE;
    inOP = FALSE;

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname)) {
        printf("ec_init on %s succeeded.\n", ifname);
        if (ec_config_init(FALSE) > 0) {
            printf("%d slaves found and configured.\n", ec_slavecount);

//            os=sizeof(ob2); ob2 = 1;
//            printf("result: %d \n",ec_SDOwrite(2,0x0f00,0x00,FALSE,os,&ob2,EC_TIMEOUTRXM));

            printf("%d\n", ec_slave[1].state);
            ec_config_map(&IOmap);
            ec_configdc();

            printf("segments : %d : %d %d %d %d\n", ec_group[0].nsegments, ec_group[0].IOsegment[0],
                   ec_group[0].IOsegment[1], ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);
            printf("Slaves mapped, state to SAFE_OP.\n");
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;

            printf("Calculated workcounter %d\n", expectedWKC);
            ec_send_processdata();
            wkc = ec_receive_processdata(EC_TIMEOUTRET);
            ec_writestate(0);
            /* wait for all slaves to reach OP state */
            ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
            inOP = TRUE;
//            printf("%d\n",ec_slave[1].state);

            uint8 *data_ptr;
            /* Get the IO map pointer from the ec_slave struct */
            data_ptr = ec_slave[2].outputs;
            /* Move pointer to correct module index */
//            data_ptr = data_ptr * 2;
            *data_ptr++ = (255) & 0xFF;
            *data_ptr++ = (255) & 0xFF;

//            *ec_slave[2].outputs=255 ;

//            for(int n =8; n <=16;n++){
//                set_output_bit(2, n, 9);
//            }

//            set_output_bit(2, 9, 1);
//            printf(" ");
//            set_output_bit(2, 8, 1);
//            set_output_bit(2, 2, 0);
//            set_output_bit(2, 3, 1);
//            set_output_bit(2, 4, 0);
//            set_output_bit(2, 5, 1);
//            set_output_bit(2, 6, 0);
//            set_output_bit(2, 7, 1);
//            set_output_bit(2, 8, 1);

            int i;
            for (i = 1; i <= 1000000; i++) {
                ec_send_processdata();
                wkc = ec_receive_processdata(EC_TIMEOUTRET);

                if (wkc >= expectedWKC) {
                    printf("Processdata cycle %4d, WKC %d", i, wkc);
                    printf(" T:%"PRId64"\r", ec_DCtime);
                    needlf = TRUE;
                }
                osal_usleep(10000);

            }
            inOP = FALSE;
        }

        printf("End simple test, close socket\n");
        /* stop SOEM, close socket */
        ec_close();
    } else {
        printf("No socket connection on %s\nExecute as root\n", ifname);
    }
}

OSAL_THREAD_FUNC ecatcheck(void *ptr) {
    int slave;
    (void) ptr;                  /* Not used */

    while (1) {
        if (inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate)) {
            if (needlf) {
                needlf = FALSE;
                printf("\n");
            }
            /* one ore more slaves are not responding */
            ec_group[currentgroup].docheckstate = FALSE;
            ec_readstate();
            for (slave = 1; slave <= ec_slavecount; slave++) {
                if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL)) {
                    ec_group[currentgroup].docheckstate = TRUE;
                    if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR)) {
                        printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                        ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                        ec_writestate(slave);
                    } else if (ec_slave[slave].state == EC_STATE_SAFE_OP) {
                        printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                        ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        ec_writestate(slave);
                    } else if (ec_slave[slave].state > EC_STATE_NONE) {
                        if (ec_reconfig_slave(slave, EC_TIMEOUTMON)) {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d reconfigured\n", slave);
                        }
                    } else if (!ec_slave[slave].islost) {
                        /* re-check state */
                        ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                        if (ec_slave[slave].state == EC_STATE_NONE) {
                            ec_slave[slave].islost = TRUE;
                            printf("ERROR : slave %d lost\n", slave);
                        }
                    }
                }
                if (ec_slave[slave].islost) {
                    if (ec_slave[slave].state == EC_STATE_NONE) {
                        if (ec_recover_slave(slave, EC_TIMEOUTMON)) {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d recovered\n", slave);
                        }
                    } else {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d found\n", slave);
                    }
                }
            }
            if (!ec_group[currentgroup].docheckstate)
                printf("OK : all slaves resumed OPERATIONAL.\n");
        }
        osal_usleep(10000);
    }
}


int main() {
    printf("Hello worldk\n");
    osal_thread_create(&thread1, 128000, ecatcheck, (void *) &ctime);
    setup();
    uint8 n = 1;
    printf("%d",n << 8);


//    printf("%p\n",&mem);
//    *pmem |= (1 << (3 - 1 + 0));
//    printf("%p\n",pmem);
//    printf("End program\n");
//        delay(1000);


//    while (1) {
//        setup();
//        delay(10000);
//    }


//    int a = 2,b =4;
//    int* ptr = Add(&a,&b);
//    printf("%p", ptr);
//    printf("%d", *ptr);
//
//    uint8 n = 45;
//    printf("%lu", sizeof(n));
    return 0;
}



//            *ec_slave[2].outputs += *ec_slave[2].outputs ;
//            printf("output: %hhu\n",*ec_slave[2].outputs);
//
//
//
//            printf("result: %d",ec_slave[2].Istartbit);
//            printf("result: %d\n",ec_slave[2].Ostartbit);


//            os=sizeof(ob2); ob2 = 1;
//            printf("result: %d ",ec_SDOwrite(2,0x0f01,0x00,FALSE,os,&ob2,EC_TIMEOUTRXM));
//
//            uint8_t rdvar = 1;
//            int rdl = sizeof(rdvar);
//
//
//            set_output_bit(2,2,1);

//            printf("segments : %d : %d %d %d %d\n", ec_group[0].nsegments, ec_group[0].IOsegment[0],
//                   ec_group[0].IOsegment[1], ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);
//
//
//            uint8_t rdvar = 1;
//            int rdl = sizeof(rdvar);
////            printf("result: %d ",ec_SDOwrite(2,0x7000,0x01,FALSE,rdl,&rdvar,EC_TIMEOUTRXM));
//
//            printf("result: %d ",ec_RxPDO(2,0x1600,rdl,&rdvar));


//            int retval ;
//            retval = 0;
//
////            uint16 rdat;
////            int rdl;
////            rdl = sizeof(rdat);
////            rdat = 0;
////            rdat = etohs(rdat);
////            retval += ec_SDOread(1, ECT_SDO_SMCOMMTYPE, (uint8) 0x00, FALSE, &rdl, &rdat, EC_TIMEOUTRXM);
////            printf(" %-12s %s\n", dtype2string(OElist.DataType[obj_subidx]), OElist.Name[obj_subidx]);
//
//
//////            ec_slave[2].PO2SOconfigx = ;
////
//
//            boolean bool = TRUE;
//            retval += ec_SDOwrite(2, 0x1601, 0x00, FALSE, 1, &bool, EC_TIMEOUTRXM);
//
////             ec_RxPDO(2, 0x1601, 1, &bool);
////            wkc = ec_SDOread(slave, idx, 0x00, FALSE, &rdl, &subcnt, EC_TIMEOUTRXM);
//
//////            uint16 h =2 ;
//            printf("retval = %d\n", retval);
//            ec_send_processdata();
//            wkc = ec_receive_processdata(EC_TIMEOUTRET);
//            printf("%d",wkc);
