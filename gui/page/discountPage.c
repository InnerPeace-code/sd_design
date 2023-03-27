#include "design/linkedList.h"
#include "design/operation.h"
#include "design/table.h"
#include "design/time.h"
#include "design/journalService.h"
#include "design/judgeService.h"
#include "design/saleService.h"
#include "design/utils.h"
#include "../../config.h"
#include "design/layout.h"
#include "design/mainWindow.h"
#include <stddef.h>
#include <malloc.h>

void DiscountDelete(int ok, void *parameter)
{
    MessageBoxCallback(ok, parameter);
    if (ok == 0)
    {
        return;
    }

    struct Data *data = parameter;

    int hasPermission;
    Judge(data->id, &hasPermission, data->password, OP_DELETE_DISCOUNT);
    if (!hasPermission)
    {
        data->messageCallback = MessageBoxCallback;
        data->message = CloneString("缺少权限：删除折扣");
        return;
    }

    LinkedList *now = data->discountCheckList->next;
    LinkedList *rowNow = data->discountTable->rows->next;
    while (now != NULL)
    {
        if (*(int *)now->data == 1)
        {
            char *id = GetRowItemByColumnName(data->discountTable, rowNow->data, "折扣编号");

            TableRow *row = NewTableRow();
            AppendTableRow(row, "折扣编号");
            Table *table = NewTable(row, NULL);

            row = NewTableRow();
            AppendTableRow(row, id);
            AppendTable(table, row);

            AddJournal(table, data->id, OP_DELETE_DISCOUNT);
            Table *response = RemoveDiscount(table);
            FreeTable(table);

            if (response != NULL)
            {
                int error = 0;
                if (response->remark != NULL && response->remark[0] != '\0')
                {
                    data->messageCallback = MessageBoxCallback;
                    data->message = CloneString(response->remark);
                    error = 1;
                }
                FreeTable(response);
                if (error)
                {
                    return;
                }
            }
        }
        now = now->next;
        rowNow = rowNow->next;
    }
    data->messageCallback = MessageBoxCallback;
    data->message = CloneString("删除成功");
}

void ConfirmDiscountDelete(struct Data *data)
{
    data->messageCallback = DiscountDelete;
    data->message = CloneString("是否确认要删除选中的折扣条目");
}

void SendDiscountRequest(struct Data *data)
{
    int hasPermission;
    Judge(data->id, &hasPermission, data->password, OP_READ_DISCOUNT);
    if (!hasPermission)
    {
        data->messageCallback = MessageBoxCallback;
        data->message = CloneString("缺少权限：读取折扣");
        return;
    }

    AddJournal(NULL, data->id, OP_READ_DISCOUNT);
    Table *response = GetAllDiscount(NULL);
    if (response != NULL)
    {
        if (response->remark != NULL && response->remark[0] != '\0')
        {
            data->messageCallback = MessageBoxCallback;
            data->message = CloneString(response->remark);
        }

        free(data->discountProperties);
        FreeTable(data->discountTable);
        FreeList(data->discountCheckList);
        data->discountCheckList = NewCheckList();
        data->discountTable = response;
        data->discountProperties = NULL;
    }
    else
    {
        data->messageCallback = MessageBoxCallback;
        data->message = CloneString("查询失败: 响应为NULL");
    }
}

void DiscountLookup(struct Data *data)
{
    LinkedList *now = data->discountCheckList->next;
    LinkedList *rowNow = data->discountTable->rows->next;
    while (now != NULL)
    {
        if (*(int *)now->data == 1)
        {
            TableRow *titleRow = CloneRow(GetTableTitle(data->discountTable));
            Table *table = NewTable(titleRow, "");
            AppendTable(table, CloneRow(rowNow->data));
            PushWindow(NewDiscountDetail("折扣详情", table));
            FreeTable(table);
            return;
        }
        now = now->next;
        rowNow = rowNow->next;
    }
    data->messageCallback = MessageBoxCallback;
    data->message = CloneString("请选择一个折扣条目");
}

void DiscountAdd(struct Data *data)
{
    LinkedList *now = data->itemCheckList->next;
    LinkedList *rowNow = data->itemTable->rows->next;
    while (now != NULL)
    {
        if (*(int *)now->data == 1)
        {
            TableRow *row = NewTableRow();
            AppendTableRow(row, "商品编号");
            AppendTableRow(row, "商品名称");
            AppendTableRow(row, "折扣比率");
            AppendTableRow(row, "客户等级");
            AppendTableRow(row, "年");
            AppendTableRow(row, "月");
            AppendTableRow(row, "日");
            AppendTableRow(row, "时");
            AppendTableRow(row, "分");
            AppendTableRow(row, "秒");
            Table *table = NewTable(row, "");

            row = NewTableRow();
            AppendTableRow(row, GetRowItemByColumnName(data->itemTable, rowNow->data, "商品编号"));
            AppendTableRow(row, GetRowItemByColumnName(data->itemTable, rowNow->data, "商品名称"));
            AppendTableRow(row, "100");
            AppendTableRow(row, "1");
            AppendTableRow(row, "");
            AppendTableRow(row, "");
            AppendTableRow(row, "");
            AppendTableRow(row, "");
            AppendTableRow(row, "");
            AppendTableRow(row, "");
            AppendTable(table, row);
            PushWindow(NewDiscountEdit("折扣编辑", data->id, data->password, table, 0));
            FreeTable(table);
            return;
        }
        now = now->next;
        rowNow = rowNow->next;
    }
    data->messageCallback = MessageBoxCallback;
    data->message = CloneString("请在商品页面选择一个商品条目");
}

void DiscountModify(struct Data *data)
{
    LinkedList *now = data->discountCheckList->next;
    LinkedList *rowNow = data->discountTable->rows->next;
    while (now != NULL)
    {
        if (*(int *)now->data == 1)
        {
            TableRow *row = NewTableRow();
            AppendTableRow(row, "折扣编号");
            AppendTableRow(row, "商品编号");
            AppendTableRow(row, "商品名称");
            AppendTableRow(row, "折扣比率");
            AppendTableRow(row, "客户等级");
            AppendTableRow(row, "年");
            AppendTableRow(row, "月");
            AppendTableRow(row, "日");
            AppendTableRow(row, "时");
            AppendTableRow(row, "分");
            AppendTableRow(row, "秒");
            Table *table = NewTable(row, NULL);

            row = NewTableRow();
            AppendTableRow(row, GetRowItemByColumnName(data->discountTable, rowNow->data, "折扣编号"));
            AppendTableRow(row, GetRowItemByColumnName(data->discountTable, rowNow->data, "商品编号"));
            AppendTableRow(row, GetRowItemByColumnName(data->discountTable, rowNow->data, "商品名称"));
            AppendTableRow(row, GetRowItemByColumnName(data->discountTable, rowNow->data, "折扣比率"));
            AppendTableRow(row, GetRowItemByColumnName(data->discountTable, rowNow->data, "客户等级"));

            const char *time = GetRowItemByColumnName(data->discountTable, rowNow->data, "截止时间");
            TimeInfo info = ParseTime(time, 0);
            free(AppendTableRow(row, LongLongToString(info.year)));
            free(AppendTableRow(row, LongLongToString(info.month)));
            free(AppendTableRow(row, LongLongToString(info.day)));
            free(AppendTableRow(row, LongLongToString(info.hour)));
            free(AppendTableRow(row, LongLongToString(info.minute)));
            free(AppendTableRow(row, LongLongToString(info.second)));

            AppendTable(table, row);
            PushWindow(NewDiscountEdit("折扣编辑", data->id, data->password, table, 1));
            FreeTable(table);
            return;
        }
        now = now->next;
        rowNow = rowNow->next;
    }
    data->messageCallback = MessageBoxCallback;
    data->message = CloneString("请选择一个折扣条目");
}

void DiscountPageLayout(struct nk_context *context, struct Window *window)
{
    struct Data *data = window->data;
    DrawMessageBox(context, "", data->message != NULL, data->message, data->messageCallback, data);

    // title
    nk_layout_row_dynamic(context, 0, 1);
    {
        if (nk_style_push_font(context, &fontLarge->handle))
        {
            nk_label(context, "折扣", NK_TEXT_LEFT);
            nk_style_pop_font(context);
        }
    }

    // filter
    nk_layout_row_begin(context, NK_STATIC, 35, 5);
    {
        nk_layout_row_push(context, 100);
        {
            nk_label(context, "通过条件 ", NK_TEXT_LEFT);
        }

        int columnCount;
        {
            TableRow *row = data->discountTable == NULL ? NULL : GetTableTitle(data->discountTable);
            columnCount = row == NULL ? 0 : row->columnCount;
            if (data->discountProperties == NULL)
            {
                data->discountProperties = malloc((columnCount + 1) * sizeof(char *));
                data->discountProperties[0] = "无";

                LinkedList *rowNow = row == NULL ? NULL : row->items;
                for (int i = 1; i < columnCount + 1; i++)
                {
                    data->discountProperties[i] = rowNow->data;
                    rowNow = rowNow->next;
                }
            }
        }

        nk_layout_row_push(context, 140);
        {
            if (nk_style_push_font(context, &fontSmall->handle))
            {
                nk_combobox(context, data->discountProperties, columnCount + 1, &data->discountPropertySelected, 35,
                        nk_vec2(200, 400));
                nk_style_pop_font(context);
            }
        }

        nk_layout_row_push(context, 30);
        {
            nk_label(context, "为", NK_TEXT_CENTERED);
        }

        nk_layout_row_push(context, 200);
        {
            nk_edit_string_zero_terminated(context,
                    (NK_EDIT_BOX | NK_EDIT_AUTO_SELECT | NK_EDIT_CLIPBOARD) & ~NK_EDIT_MULTILINE,
                    data->discountValueBuffer, BUFFER_SIZE * sizeof(char), nk_filter_default);
        }

        nk_layout_row_push(context, 100);
        {
            nk_label(context, "进行筛选", NK_TEXT_LEFT);
        }

        nk_layout_row_end(context);
    }

    nk_layout_row_static(context, 10, 0, 0);

    OperationLayout(context, OP_TYPE_GET | OP_TYPE_DETAIL | OP_TYPE_ADD | OP_TYPE_DELETE | OP_TYPE_UPDATE, (OperationHandler)SendDiscountRequest,
            (OperationHandler)DiscountLookup, (OperationHandler)DiscountAdd, (OperationHandler)ConfirmDiscountDelete, (OperationHandler)DiscountModify,data);

    nk_layout_row_dynamic(context, 10, 1);
    {
        struct nk_rect space;
        nk_widget(&space, context);
        struct nk_command_buffer *canvas = nk_window_get_canvas(context);
        nk_stroke_line(canvas, space.x, space.y + space.h / 2, space.x + space.w, space.y + space.h / 2, 1,
                nk_rgb(100, 100, 100));
    }

    char *from, *to;
    DateRangeFilterLayout(context, "筛选截止时间", &from, &to);

    nk_layout_row_dynamic(context, 10, 1);
    {
        struct nk_rect space;
        nk_widget(&space, context);
        struct nk_command_buffer *canvas = nk_window_get_canvas(context);
        nk_stroke_line(canvas, space.x, space.y + space.h / 2, space.x + space.w, space.y + space.h / 2, 1,
                nk_rgb(100, 100, 100));
    }

    nk_layout_row_dynamic(context, nk_window_get_height(context) - 285, 1);
    {
        if (nk_style_push_font(context, &fontSmall->handle))
        {
            if (nk_group_begin(context, "查询结果", NK_WINDOW_BORDER))
            {
                TableLayout(context, data->discountTable, data->discountCheckList,
                        data->discountPropertySelected == 0
                        ? NULL
                        : data->discountProperties[data->discountPropertySelected],
                        data->discountValueBuffer,
                        "截止时间",
                        from,
                        to);
                nk_group_end(context);
            }

            nk_style_pop_font(context);
        }
    }
}
