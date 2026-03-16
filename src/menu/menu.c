/*
 * menu.c  —— 表示层：用户界面与交互
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/card.h"
#include "../../include/billing.h"
#include "../../include/rate.h"
#include "../../include/tool.h"
#include "../../include/card_service.h"
#include "../../include/billing_service.h"
#include "../../include/record_service.h"
#include "../../include/rate_service.h"
#include "../../include/menu.h"

/* ================================================================
 *  主菜单输出
 * ================================================================ */
void outputMenu(void)
{
    printf("1025005072李宇轩计算机类2502\n");
    printf("\t============================================\n");
    printf("\t           计  费  管  理  系  统          \n");
    printf("\t============================================\n");
    printf("\t  1. 添加卡                                \n");
    printf("\t  2. 查询卡                                \n");
    printf("\t  3. 上机                                  \n");
    printf("\t  4. 下机                                  \n");
    printf("\t  5. 充值                                  \n");
    printf("\t  6. 退费                                  \n");
    printf("\t  7. 查询统计                              \n");
    printf("\t  8. 注销卡                                \n");
    // printf("\t  9. 计费标准                              \n");
    printf("\t  0. 退出                                  \n");
    printf("\t============================================\n");
    printf("\t请选择菜单项编号: ");
}

/* ================================================================
 *  1. 添加卡
 * ================================================================ */
void addCardMenu(void)
{
    printf("\n===== 添加卡 =====\n");

    Card card;
    memset(&card, 0, sizeof(Card));

    printf("请输入卡号  (最多18位): ");
    if (scanf("%18s", card.cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            clearInputBuf();
            printf("卡号长度不能超过18位！\n");
            return;
        }
    }
    if (strlen(card.cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码  (最多 8位): ");
    if (scanf("%8s", card.password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            clearInputBuf();
            printf("密码长度不能超过8位！\n");
            return;
        }
    }
    if (strlen(card.password) == 0) {
        printf("密码为空！\n");
        return;
    }
    printf("请输入开卡金额        : ");
    if (scanf("%lf", &card.money) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }

    int ret = addCard(&card);
    if (ret == 1) {
        printf("\n\t%-20s%-12s%-12s%-10s\n",
               "卡号", "密码", "余额", "卡状态");
        printf("\t--------------------------------------------------\n");
        printf("\t%-20s%-12s%-12.2f%-10s\n",
               card.cardNo, card.password, card.money, "未上机");
        printf("添加卡成功！\n");
    } else if (ret == -1) {
        printf("添加卡失败！该卡号已存在。\n");
    } else {
        printf("添加卡失败！写入文件时出错。\n");
    }
}

/* ================================================================
 *  2. 查询卡
 * ================================================================ */
void queryCardMenu(void)
{
    printf("\n===== 查询卡 =====\n");
    printf("请输入卡号关键字: ");

    char keyword[CARD_NO_LEN];
    if (scanf("%18s", keyword) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    {
        int ch = getchar();
        if (ch != '\n' && ch != EOF) {
            clearInputBuf();
            printf("卡号长度不能超过18位！\n");
            return;
        }
    }
    if (strlen(keyword) == 0) {
        printf("卡号为空！\n");
        return;
    }

    CardList list, result;
    getCard(&list);
    queryCards(&list, keyword, &result);

    if (result.count == 0) {
        printf("未找到相关卡信息。\n");
    } else {
        const char *stateNames[] = {"未上机", "上机中", "已注销"};
        printf("\n\t%-20s%-12s%-12s%-10s\n",
               "卡号", "密码", "余额", "卡状态");
        printf("\t--------------------------------------------------\n");
        CardNode *p = result.head;
        while (p != NULL) {
            Card *c = &p->data;
            int si = (c->state >= 0 && c->state <= 2) ? c->state : 0;
            printf("\t%-20s%-12s%-12.2f%-10s\n",
                   c->cardNo, c->password, c->money, stateNames[si]);
            p = p->next;
        }
    }

    freeCardList(&result);
    freeCardList(&list);
}

/* ================================================================
 *  3. 上机
 * ================================================================ */
void loginMenu(void)
{
    printf("\n===== 上机 =====\n");

    /* 第1步：展示所有可用计费方案 */
    int rateCount = 0;
    Rate *plans = getAllRatePlans(&rateCount);
    if (plans == NULL || rateCount == 0) {
        printf("暂无可用计费方案，请先在【计费标准】菜单中添加方案。\n");
        if (plans != NULL) free(plans);
        return;
    }

    printf("当前可用计费方案：\n");
    printf("\t%-6s%-34s%-14s\n", "编号", "方案名称", "费率(元/时)");
    printf("\t----------------------------------------------\n");
    for (int i = 0; i < rateCount; i++)
        printf("\t%-6d%-34s%-14.2f\n",
               plans[i].rateId, plans[i].name, plans[i].ratePerHour);

    /* 第2步：用户选择方案 */
    int rateId;
    printf("请输入要使用的方案编号: ");
    if (scanf("%d", &rateId) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        free(plans);
        return;
    }

    /* 查找选中方案的费率 */
    double selectedRate = 0.0;
    for (int i = 0; i < rateCount; i++) {
        if (plans[i].rateId == rateId) {
            selectedRate = plans[i].ratePerHour;
            break;
        }
    }
    free(plans);

    if (selectedRate <= 0.0) {
        printf("方案编号不存在，请重新选择！\n");
        return;
    }

    /* 第3步：输入卡号和密码 */
    char cardNo[CARD_NO_LEN];
    char password[PASSWORD_LEN];
    printf("请输入卡号: ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        free(plans);
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        free(plans);
        return;
    }
    printf("请输入密码: ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }

    Card card;
    int ret = loginCard(cardNo, password, selectedRate, &card);
    if (ret == 1) {
        char timeBuf[32];
        timeToString(card.lastUseTime, timeBuf, sizeof(timeBuf));
        printf("\n\t%-20s%-12s%-22s%-14s\n", "卡号", "余额", "上机时间", "费率(元/时)");
        printf("\t------------------------------------------------------------------\n");
        printf("\t%-20s%-12.2f%-22s%-14.2f\n",
               card.cardNo, card.money, timeBuf, card.currentRate);
        printf("上机成功！\n");
    } else if (ret == -2) {
        printf("该卡正在使用或已注销！\n");
    } else if (ret == -3) {
        printf("余额不足！\n");
    } else if (ret == -4) {
        printf("上机失败！费率无效。\n");
    } else {
        printf("上机失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  4. 下机
 * ================================================================ */
void logoutMenu(void)
{
    printf("\n===== 下机 =====\n");

    char cardNo[CARD_NO_LEN];
    char password[PASSWORD_LEN];
    printf("请输入卡号: ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码: ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }

    double cost    = 0.0;
    time_t endTime = 0;
    int ret = logoutCard(cardNo, password, &cost, &endTime);
    if (ret == 1) {
        char timeBuf[32];
        timeToString(endTime, timeBuf, sizeof(timeBuf));
        printf("\n\t%-20s%-12s%-22s\n", "卡号", "消费金额", "下机时间");
        printf("\t--------------------------------------------------\n");
        printf("\t%-20s%-12.2f%-22s\n", cardNo, cost, timeBuf);
        printf("下机成功！\n");
    } else if (ret == -2) {
        printf("下机失败！该卡未处于上机状态。\n");
    } else {
        printf("下机失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  5. 充值
 * ================================================================ */
void rechargeMenu(void)
{
    printf("\n===== 充值 =====\n");

    char   cardNo[CARD_NO_LEN];
    char   password[PASSWORD_LEN];
    double amount;

    printf("请输入卡号  : ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码  : ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }
    printf("请输入充值金额: ");
    if (scanf("%lf", &amount) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }

    if (amount <= 0.0) {
        printf("充值金额必须大于0！\n");
        return;
    }

    Card card;
    int ret = rechargeCard(cardNo, password, amount, &card);
    if (ret == 1) {
        printf("\n\t%-20s%-12s%-12s\n", "卡号", "充值金额", "余额");
        printf("\t------------------------------------------\n");
        printf("\t%-20s%-12.2f%-12.2f\n", cardNo, amount, card.money);
        printf("充值成功！\n");
    } else if (ret == -2) {
        printf("充值失败！该卡已注销。\n");
    } else {
        printf("充值失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  6. 退费
 * ================================================================ */
void refundMenu(void)
{
    printf("\n===== 退费 =====\n");

    char   cardNo[CARD_NO_LEN];
    char   password[PASSWORD_LEN];
    double amount;

    printf("请输入卡号  : ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码  : ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }
    printf("请输入退费金额: ");
    if (scanf("%lf", &amount) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }

    if (amount <= 0.0) {
        printf("退费金额必须大于0！\n");
        return;
    }

    Card card;
    int ret = refundCard(cardNo, password, amount, &card);
    if (ret == 1) {
        printf("\n\t%-20s%-12s%-12s\n", "卡号", "退费金额", "余额");
        printf("\t------------------------------------------\n");
        printf("\t%-20s%-12.2f%-12.2f\n", cardNo, amount, card.money);
        printf("退费成功！\n");
    } else if (ret == -2) {
        printf("退费失败！该卡正在使用或已注销。\n");
    } else if (ret == -3) {
        printf("退费失败！退费金额超过余额。\n");
    } else {
        printf("退费失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  7. 查询统计
 * ================================================================ */
void queryStatisticsMenu(void)
{
    printf("\n===== 查询统计 =====\n");
    printf("  1. 查询消费记录\n");
    printf("  2. 统计总营业额\n");
    printf("请选择: ");

    int choice;
    scanf("%d", &choice);

    int count = 0;
    Billing *billings = queryBillings(&count);   /* 通过业务逻辑层获取 */

    if (choice == 1) {
        if (billings == NULL || count == 0) {
            printf("暂无消费记录。\n");
        } else {
            printf("\n\t%-20s%-22s%-22s%-12s%-10s\n",
                   "卡号", "上机时间", "下机时间", "消费金额", "状态");
            printf("\t----------------------------------------------------------------------\n");
            for (int i = 0; i < count; i++) {
                char sBuf[32], eBuf[32];
                timeToString(billings[i].startTime, sBuf, sizeof(sBuf));
                timeToString(billings[i].endTime,   eBuf, sizeof(eBuf));
                printf("\t%-20s%-22s%-22s%-12.2f%-10s\n",
                       billings[i].cardNo, sBuf, eBuf,
                       billings[i].amount,
                       billings[i].state == BILLING_STATE_SETTLED ? "已结算" : "未结算");
            }
        }
    } else if (choice == 2) {
        double total = getTotalRevenue();         /* 通过业务逻辑层统计 */
        printf("\n总营业额：%.2f 元\n", total);
    } else {
        printf("无效的选择！\n");
    }

    if (billings != NULL) free(billings);
}

/* ================================================================
 *  8. 注销卡
 * ================================================================ */
void cancelCardMenu(void)
{
    printf("\n===== 注销卡 =====\n");

    char cardNo[CARD_NO_LEN];
    char password[PASSWORD_LEN];
    printf("请输入卡号: ");
    if (scanf("%18s", cardNo) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(cardNo) == 0) {
        printf("卡号为空！\n");
        return;
    }
    printf("请输入密码: ");
    if (scanf("%8s", password) != 1) {
        clearInputBuf();
        printf("输入无效！\n");
        return;
    }
    if (strlen(password) == 0) {
        printf("密码为空！\n");
        return;
    }

    double refundAmount = 0.0;
    int ret = cancelCardService(cardNo, password, &refundAmount);
    if (ret == 1) {
        printf("\n\t%-20s%-12s\n", "卡号", "退款金额");
        printf("\t--------------------------------\n");
        printf("\t%-20s%-12.2f\n", cardNo, refundAmount);
        printf("注销卡成功！\n");
    } else if (ret == -2) {
        printf("注销卡失败！该卡正在使用中，请先下机。\n");
    } else if (ret == -3) {
        printf("注销卡失败！该卡已注销。\n");
    } else {
        printf("注销卡失败！卡号或密码错误。\n");
    }
}

/* ================================================================
 *  9. 计费标准管理
 * ================================================================ */
void rateManagementMenu(void)
{
    int choice;
    do {
        printf("\n===== 计费标准管理 =====\n");
        printAllRates();
        printf("\n  1. 添加计费方案\n");
        printf("  2. 删除计费方案\n");
        printf("  0. 返回\n");
        printf("请选择: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuf();
            choice = -1;
        }

        if (choice == 1) {
            char name[RATE_NAME_LEN];
            double newRate;
            printf("请输入方案名称（最多32字符）: ");
            scanf(" %32[^\n]", name);
            printf("请输入费率（元/小时）: ");
            scanf("%lf", &newRate);
            if (newRate <= 0.0) {
                printf("费率必须大于 0！\n");
            } else {
                if (addRatePlan(name, newRate))
                    printf("计费方案「%s」（%.2f 元/时）已添加。\n", name, newRate);
                else
                    printf("添加失败！写入文件时出错。\n");
            }
        } else if (choice == 2) {
            int rateId;
            printf("请输入要删除的方案编号: ");
            if (scanf("%d", &rateId) != 1) {
                clearInputBuf();
                printf("输入无效！\n");
                continue;
            }
            if (deleteRatePlan(rateId))
                printf("方案编号 %d 已删除。\n", rateId);
            else
                printf("删除失败！未找到编号为 %d 的方案。\n", rateId);
        }
        clearInputBuf();
    } while (choice != 0);
}
