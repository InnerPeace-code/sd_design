#include "../../data/operation.h"
#include "../../services/customerService.h"
#include "../../services/journalService.h"
#include "../../services/judgeService.h"
#include "../../utils.h"
#include "../config.h"
#include "../layout.h"
#include <malloc.h>
#include <string.h>

struct Data
{
    struct Table *customer;
    int id;
    const char *password;
    char *message;
    int modify;
    Window *window;

    void (*messageCallback)(int, void *);
};

static void MessageBoxCallBack(__attribute__((unused)) int ok, void *parameter)
{
    struct Data *data = parameter;
    free(data->message);
    data->message = NULL;
}

static void FinishCallback(__attribute__((unused)) int ok, void *parameter)
{
    MessageBoxCallBack(ok, parameter);
    struct Data *data = parameter;
    data->window->isClosed = 1;
}

static void SendRequest(struct Data *data)
{
    int hasPermission;
    Operation operation = data->modify ? OP_UPDATE_CUSTOMER : OP_ADD_CUSTOMER;
    Judge(data->id, &hasPermission, data->password, operation);
    if (!hasPermission)
    {
        data->messageCallback = FinishCallback;
        data->message = CloneString("没有权限");
        return;
    }

    TableRow *row = NewTableRow();
    if (data->modify)
    {
        AppendTableRow(row, "客户编号");
    }
    AppendTableRow(row, "客户等级");
    AppendTableRow(row, "客户姓名");
    AppendTableRow(row, "客户联系方式");
    Table *request = NewTable(row, NULL);

    row = NewTableRow();
    TableRow *sourceRow = GetRowByIndex(data->customer, 1);
    if (data->modify)
    {
        AppendTableRow(row, GetRowItemByColumnName(data->customer, sourceRow, "客户编号"));
    }
    AppendTableRow(row, GetRowItemByColumnName(data->customer, sourceRow, "客户等级"));
    AppendTableRow(row, GetRowItemByColumnName(data->customer, sourceRow, "客户姓名"));
    AppendTableRow(row, GetRowItemByColumnName(data->customer, sourceRow, "客户联系方式"));
    AppendTable(request, row);

    Table *response;
    if (data->modify)
    {
        AddJournal(request, data->id, OP_UPDATE_CUSTOMER);
        response = UpdateCustomer(request);
    }
    else
    {
        AddJournal(request, data->id, OP_ADD_CUSTOMER);
        response = AddCustomer(request);
    }
    FreeTable(request);

    if (response != NULL && response->remark != NULL && response->remark[0] != '\0')
    {
        data->messageCallback = MessageBoxCallBack;
        data->message = CloneString(response->remark);
    }
    else
    {
        data->messageCallback = FinishCallback;
        data->message = CloneString("操作成功完成");
    }

    if (response != NULL)
    {
        FreeTable(response);
    }
}

void CustomerEditLayout(struct nk_context *context, Window *window)
{
    struct Data *data = window->data;
    DrawMessageBox(context, "", data->message != NULL, data->message, data->messageCallback, data);
    TableRow *dataRow = GetRowByIndex(data->customer, 1);

    nk_style_push_font(context, &fontLarge->handle);
    {
        nk_layout_row_dynamic(context, 0, 1);
        nk_label(context, "客户编辑", NK_TEXT_CENTERED);

        nk_style_pop_font(context);
    }

    nk_style_push_font(context, &fontSmall->handle);
    {
        nk_layout_row_begin(context, NK_STATIC, 0, 2);
        {
            nk_style_push_font(context, &fontMedium->handle);
            {
                nk_layout_row_push(context, 100);
                nk_label(context, "客户等级", NK_TEXT_CENTERED);
                nk_style_pop_font(context);
            }
            nk_layout_row_push(context, 100);
            nk_edit_string_zero_terminated(
                    context, (NK_EDIT_BOX | NK_EDIT_CLIPBOARD | NK_EDIT_AUTO_SELECT) & (~NK_EDIT_MULTILINE),
                    GetRowItemByColumnName(data->customer, dataRow, "客户等级"), 512, nk_filter_decimal);

            nk_layout_row_end(context);
        }

        nk_layout_row_begin(context, NK_STATIC, 0, 2);
        {
            nk_style_push_font(context, &fontMedium->handle);
            {
                nk_layout_row_push(context, 100);
                nk_label(context, "客户姓名", NK_TEXT_CENTERED);
                nk_style_pop_font(context);
            }
            nk_layout_row_push(context, 300);
            nk_edit_string_zero_terminated(
                    context, (NK_EDIT_BOX | NK_EDIT_CLIPBOARD | NK_EDIT_AUTO_SELECT) & (~NK_EDIT_MULTILINE),
                    GetRowItemByColumnName(data->customer, dataRow, "客户姓名"), 512, nk_filter_default);

            nk_layout_row_end(context);
        }

        nk_layout_row_begin(context, NK_STATIC, 0, 2);
        {
            nk_style_push_font(context, &fontMedium->handle);
            {
                nk_layout_row_push(context, 150);
                nk_label(context, "客户联系方式", NK_TEXT_CENTERED);
                nk_style_pop_font(context);
            }
            nk_layout_row_push(context, 300);
            nk_edit_string_zero_terminated(
                    context, (NK_EDIT_BOX | NK_EDIT_CLIPBOARD | NK_EDIT_AUTO_SELECT) & (~NK_EDIT_MULTILINE),
                    GetRowItemByColumnName(data->customer, dataRow, "客户联系方式"), 512, nk_filter_default);

            nk_layout_row_end(context);
        }

        PlaceNothing(context);
        nk_layout_row_dynamic(context, 0, 5);
        {
            PlaceNothing(context);
            if (nk_button_label(context, "确定"))
            {
                SendRequest(data);
            }
            PlaceNothing(context);
            if (nk_button_label(context, "取消"))
            {
                window->isClosed = 1;
            }
        }

        nk_style_pop_font(context);
    }
}

void FreeCustomerEdit(Window *window)
{
    struct Data *data = window->data;
    FreeTable(data->customer);
    free(window);
}

Window *NewCustomerEdit(const char *title, int id, const char *password, Table *customer, int modify)
{
    Window *window = malloc(sizeof(Window));
    window->isClosed = 0;
    window->layoutFunc = CustomerEditLayout;
    window->freeFunc = FreeCustomerEdit;
    window->title = title;

    struct Data *data = malloc(sizeof(struct Data));
    memset(data, 0, sizeof(struct Data));
    data->customer = CloneTableBuffered(customer, 512);
    data->id = id;
    data->password = password;
    data->modify = modify;
    data->window = window;

    window->data = data;
    window->next = NULL;

    return window;
}
