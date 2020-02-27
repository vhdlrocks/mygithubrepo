
"""
First, go to powertochoose.org, put in your zip code, and finally at the bottom right hand side of the screen
click the button to "Export results to Excel". Save the file to the same folder that this python file is at using
the default name power-to-choose.xlxs.
"""

import xlrd
from tabulate import tabulate
#----------------------------------------------------------------------
def open_file_example(path):
    """
    Open and read an Excel file
    """
    book = xlrd.open_workbook(path)
    
    # print number of sheets
    print('Number of sheets: {}'.format(book.nsheets))
    
    # print sheet names
    print('Sheet names: {}'.format(book.sheet_names()))
    
    # get the first worksheet
    first_sheet = book.sheet_by_index(0)
    
    # read a row
    print('Read first row: {}'.format(first_sheet.row_values(0)))
    
    # read a cell
    cell = first_sheet.cell(0,0)
    print('First sheet cell 0,0: {}'.format(cell))
    print('First sheet cell 0,0 value: {}'.format(cell.value))
    
    # read a row slice
    print('First sheet, row slice val: {}'.format(first_sheet.row_slice(rowx=0,
                                start_colx=0,
                                end_colx=2)))
    
    # Check number of rows
    numRows = first_sheet.nrows
    numCols = first_sheet.ncols
    print('Rows: {}, Cols: {}'.format(numRows, numCols))

    print(type(first_sheet))

# def withUsageFindBestPrice(fname):
#     """
#     Open and read an Excel file
#     """
#     book = xlrd.open_workbook(path)



#----------------------------------------------------------------------
if __name__ == "__main__":
    path = "power-to-choose.xlsx"
    # open_file_example(path)

    book = xlrd.open_workbook(path)
    p2c_sheet = book.sheet_by_index(0)
    numRows = p2c_sheet.nrows
    numCols = p2c_sheet.ncols

    estimatedUsage = 2500
    pastUsageMonths     = ['jan', 'feb', 'oct', 'nov', 'dec', 'jan', 'feb']
    pastUsage           = [3306, 2339, 2066, 2273, 1584]
    pastUsageNumDays    = [29, 31, 31, 34, 29]

    pkwh500 = 4
    pkwh1000 = 5
    pkwh2000 = 6
    kwhblks = [500, 500, 1000]
    # cell = p2c_sheet.cell(1,pkwh500)
    # print('cell value: {}, type: {}'.format(cell.value, type(cell.value)))

    # Get the cost based on current prices and estimated monthly usage
    cost_list = [0.0]
    for i in range(1,numRows):
        cost_accum = 0
        if estimatedUsage <= 500:
            cell = p2c_sheet.cell(i,pkwh500)
            cost_accum += estimatedUsage * cell.value
        elif estimatedUsage <= 1000:
            cell = p2c_sheet.cell(i,pkwh1000)
            cost_accum += estimatedUsage * cell.value
        else:
            cell = p2c_sheet.cell(i,pkwh2000)
            cost_accum += estimatedUsage * cell.value
        # for j in range(3):
        #     cell = p2c_sheet.cell(i,pkwh500+j)
        #     cost_accum += estimatedUsage * cell.value
        cost_list.append(cost_accum)
    
    # Sort the list of monthly costs
    sorted_list = sorted(cost_list)

    # Use the sorted list to match up the row from the spreadsheet so the row information can be retrieved
    high_to_low_list = []
    for i in range(1,numRows):
        index = cost_list.index(sorted_list[i])
        high_to_low_list.append(index)
        cost_list[index] = 0
    
    # Format the header and show the top N options
    N = 30
    row_vals = p2c_sheet.row_values(0)
    headers = ['Row', 'Monthly', 'Term(months)', row_vals[0], row_vals[2], row_vals[3], row_vals[4], row_vals[5], row_vals[6]]
    table = []
    for i in range(N):
        row = high_to_low_list[i]
        row_vals = p2c_sheet.row_values(row)
        table.append([(row+1), sorted_list[i+1], row_vals[9], row_vals[0], row_vals[2], row_vals[3], row_vals[4], row_vals[5], row_vals[6]])

    print('\r')
    print(tabulate(table, headers))

