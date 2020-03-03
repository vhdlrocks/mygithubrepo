"""
First, go to powertochoose.org, put in your zip code, and finally at the bottom right hand side of the screen
click the button to "Export results to Excel". Save the file to the same folder that this python file is at using
the default name power-to-choose.xlxs.
"""

import xlrd
from tabulate import tabulate
import matplotlib.pyplot as plt
import os
from os import path
from datetime import datetime



def open_file_example(path):
    """
    Summary: Open and read an Excel file

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
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



def getLstCol(list_2d, col):
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """
    col_lst = []
    for i in range(1,len(list_2d)):
        col_lst.append(list_2d[i][col])
    return col_lst



def monthYear(data):
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    monthYearLst = []
    for i in range(1,len(data)):
        monthYearLst.append('{}/{}'.format(str(data[i][0]), str(data[i][1])))
    return monthYearLst



def plotUsageHistory(data):
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    x_val = monthYear(data)
    y_val = getLstCol(data,2)
    plt.ylabel(data[0][2])
    plt.xlabel(data[0][0])
    plt.plot(x_val, y_val, label='Usage History')
    plt.show()



def runWebUtil(zipcode):
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    try:
        from selenium import webdriver
        from selenium.webdriver.common.keys import Keys
    except:
        print('Need to install selenium package')
    import time

    # Set current working directory
    cwd_path = os.getcwd()

    # Set Chrome options
    chromeOptions = webdriver.ChromeOptions()
    prefs = {"download.default_directory" : cwd_path}
    chromeOptions.add_experimental_option("prefs",prefs)

    # Must be ran from folder that contains 'p2c_func.py' and 'chromedriver_win32' folder containing 'chromedriver.exe'
    chromeDriverPath = cwd_path + '/chromedriver_win32/chromedriver.exe'
    try:
        # driver = webdriver.Chrome(executable_path=chromeDriverPath, chrome_options=chromeOptions)
        driver = webdriver.Chrome(executable_path=chromeDriverPath, options=chromeOptions)
    except:
        error_code = """
            Error:
            Must be ran from folder that contains 'p2c_func.py' and 'chromedriver_win32' folder containing 'chromedriver.exe'
            <cwd>
                p2c_func.py
                chromedriver_win32
                    chromedriver.exe

            Note: Chrome driver can be downloaded here: https://chromedriver.chromium.org/downloads
            """
        print(error_code)
        exit()

    driver.get('http://powertochoose.org')

    #homezipcode
    field_id = 'homezipcode'
    field = driver.find_element_by_id(field_id)
    field.send_keys(zipcode)
    field.send_keys(Keys.ENTER)
    field.submit()

    #view_all_results
    # time.sleep(5)
    # btn_id = 'view_all_results'
    # btn = driver.find_element_by_id(btn_id)
    # btn.click()

    #export-excel
    time.sleep(1) # Allow time for the page to update
    class_name = 'export-excel'
    btn = driver.find_element_by_class_name(class_name)
    btn.click()

    #homezipcode
    time.sleep(1) # Let the user actually see something!
    driver.quit()



def get_p2c_data(path):
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    p2c_wb = xlrd.open_workbook(path)
    p2c_sheet = p2c_wb.sheet_by_index(0)
    numRows = p2c_sheet.nrows
    numCols = p2c_sheet.ncols
    return p2c_sheet, numRows, numCols



def getPriceList(estimatedUsage, p2c_sheet, numRows):
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    pkwh500 = 4
    pkwh1000 = 5
    pkwh2000 = 6
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
    return high_to_low_list, sorted_list



def compareOp(compare_val, op, compare_val_to_this):
    """
    Summary: Supports: '==', '!=', '>', '<', '>=', '<='

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    if op == '>': # True if compare_val is greater than compare_val_to_this
        if compare_val > compare_val_to_this:
            return True
        else:
            return False

    elif op == '<': # True if compare_val is less than compare_val_to_this
        if compare_val < compare_val_to_this:
            return True
        else:
            return False

    elif op == '==': # True if compare_val is equal to compare_val_to_this
        if compare_val == compare_val_to_this:
            return True
        else:
            return False

    elif op == '!=': # True if compare_val is not equal to compare_val_to_this
        if compare_val != compare_val_to_this:
            return True
        else:
            return False

    elif op == '>=': # True if compare_val is greater than or equal to compare_val_to_this
        if compare_val >= compare_val_to_this:
            return True
        else:
            return False

    elif op == '<=': # True if compare_val is less than or equal to compare_val_to_this
        if compare_val >= compare_val_to_this:
            return True
        else:
            return False



def showBestOptions(p2c_sheet, high_to_low_list, sorted_list, N, numRows, numCols, filterLst=[]):
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    # cell = first_sheet.cell(0,0)
    # print('First sheet cell 0,0: {}'.format(cell))
    # print('First sheet cell 0,0 value: {}'.format(cell.value))

    # N = 30
    row_vals = p2c_sheet.row_values(0)
    headers = ['Row', 'Monthly', 'Term(mo)', row_vals[0], row_vals[2], row_vals[3], row_vals[4], row_vals[5], row_vals[6]]
    table = []
    skip_count = 0
    for i in range(N):
        if len(filterLst) != 0: # There are items to filter
            if isinstance(filterLst[0], list) == False: # Single filter
                temp = []
                for k in range(len(filterLst)):
                    temp.append(filterLst[k])
                filterLst = [temp]
                num_filters = 1
            else: # Multiple filters
                num_filters = len(filterLst)
            while True:
                if i+skip_count == numRows-1:
                    break
                useRow = False
                for j in range(num_filters):
                    row = high_to_low_list[i+skip_count]
                    cell = p2c_sheet.cell(row,filterLst[j][0])
                    if compareOp(cell.value, filterLst[j][1], filterLst[j][2]) == True:
                        useRow = True
                    else:
                        useRow = False
                        break

                if useRow == True:
                    break
                else:
                    skip_count += 1
        else:
            row = high_to_low_list[i]
        row_vals = p2c_sheet.row_values(row)
        if i+skip_count >= numRows-1:
            break

        table.append([(row+1), sorted_list[i+skip_count+1], row_vals[9], row_vals[0], row_vals[2], row_vals[3], row_vals[4], row_vals[5], row_vals[6]])

    print('\r')
    print(tabulate(table, headers))



def getUsageData():
    """
    Summary: <description>

    Parameters:
    <variable name> (<type>): <description>

    Returns:
    <variable name> (<type>): <description>
    """

    pastUsageData = [   [ 'month', 'year', 'usage_kwh', 'cost', 'num_days'],
                        [       6,   2018,        3394,   None,       None],
                        [       7,   2018,        3535,   None,       None],
                        [       8,   2018,        3853,   None,       None],
                        [       9,   2018,        2790,   None,       None],
                        [      10,   2018,        2035,   None,       None],
                        [      11,   2018,        1651,   None,       None],
                        [      12,   2018,        1784,   None,       None],
                        [       1,   2019,        1216,   None,       None],
                        [       2,   2019,        1350,   None,       None],
                        [       3,   2019,        1626,   None,       None],
                        [       4,   2019,        2649,   None,       None],
                        [       5,   2019,        3270,   None,       None],
                        [       6,   2019,        3907,   None,       None],
                        [       7,   2019,        2730,   None,       None],
                        [       8,   2019,        3722,   None,       None],
                        [       9,   2019,        3500,   None,       None], # Guessed
                        [      10,   2019,        3306,   None,         29],
                        [      11,   2019,        2339,   None,         31],
                        [      12,   2019,        2066,   None,         31],
                        [       1,   2020,        2273,   None,         34],
                        [       2,   2020,        1584,   None,         29]
    ]
    return pastUsageData



def main():

    # If there is already an old version of the p2c spreadsheet, store it in a directory.
    # If the directory doesn't exist, create it.
    # Apply Y/M/D and Time to old p2c spreadsheet.
    dflt_p2c_ss = "power-to-choose.xlsx"
    cwd_path = os.getcwd()
    curr_xlsx_path = cwd_path + '\\' + dflt_p2c_ss
    print(curr_xlsx_path)
    if path.exists(curr_xlsx_path):
        old_xlsx_dir_name = 'xlsx_history'
        old_xlsx_dir_path = cwd_path + '\\' + old_xlsx_dir_name
        if os.path.isdir(old_xlsx_dir_path):
            print('{} directory found'.format(old_xlsx_dir_name))
        else:
            print('Attempting to create directory {}...'.format(old_xlsx_dir_path))
            try:
                os.mkdir(old_xlsx_dir_name)
                print('Directory successfully created')
            except:
                print('Creation of directory {} failed...'.format(old_xlsx_dir_name))


        # datetime object containing current date and time
        now = datetime.now()
        dt_string = 'old_' + now.strftime('D%Y/%m/%d_T%H:%M:%S') + '.xlsx'
        dt_string = dt_string.replace('/', '_')
        dt_string = dt_string.replace(':', '_')
        print(dt_string)
        os.rename(dflt_p2c_ss, old_xlsx_dir_path + '\\' + dt_string)

    # Get powertochoose latest spreadsheet from web
    zipcode = '77494'
    runWebUtil(zipcode)

    # Get xlrd class
    p2c_sheet, numRows, numCols = get_p2c_data(dflt_p2c_ss)

    # Get sorted list of best power options based on estimaged single month kwh usage
    estimatedUsage = 2500
    high_to_low_list, sorted_list = getPriceList(estimatedUsage, p2c_sheet, numRows)

    # Print to console the first N options starting with the best
    N = 20
    colNum = 9
    op = '>'
    val = 6
    filterList = [colNum, op, val]
    # filterList = [
    #     [colNum, op, val],
    #     [10, '==', 'True']
    # ]
    showBestOptions(p2c_sheet, high_to_low_list, sorted_list, N, numRows, numCols, filterLst=filterList)

    # Plot usage data
    pastUsageData = getUsageData() # User defined data
    plotUsageHistory(pastUsageData)

    print("Done!")


if __name__ == "__main__":
    main()