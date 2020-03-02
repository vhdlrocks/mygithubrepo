
"""
First, go to powertochoose.org, put in your zip code, and finally at the bottom right hand side of the screen
click the button to "Export results to Excel". Save the file to the same folder that this python file is at using
the default name power-to-choose.xlxs.

http://powertochoose.org/en-us/Plan/Results#

https://electricityplans.com/texas/plans/?zipcode=77494&tdu=cnp&usage=2500&fwp_plan_term=1.00%2C60.00&fwp_sort=plan_rate_asc

"""

import xlrd
from tabulate import tabulate
import matplotlib.pyplot as plt

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

def getLstCol(list_2d, col):
    # col = col
    # list_2d = list_2d
    # val = len(list_2d)
    # print('val: {}'.format(val))
    # return [row[col] for row in range(1,len(list_2d))]
    col_lst = []
    for i in range(1,len(list_2d)):
        col_lst.append(list_2d[i][col])
    return col_lst

def monthYear(data):
    monthYearLst = []
    for i in range(1,len(data)):
        monthYearLst.append('{}/{}'.format(str(data[i][0]), str(data[i][1])))
    return monthYearLst

def plotUsageHistory(data):
    # x_val = getLstCol(data,0)
    # x_val_str = []
    # for i in x_val:
    #     x_val_str.append(monthYear(data))
    x_val = monthYear(data)
    y_val = getLstCol(data,2)
    plt.ylabel(data[0][2])
    plt.xlabel(data[0][0])
    # plt.plot(x_val, y_val, 'ro', label='Usage History')
    plt.plot(x_val, y_val, label='Usage History')
    # plt.bar(f[:N // 2], np.abs(fft)[:N // 2] * 1 / N, width=1.5)  # Bar graph
    # plt.plot(f[:N // 2], np.abs(fft)[:N // 2] * 1 / N)  # Smooth curves graph
    # plt.legend(loc='best')
    plt.show()

def runWebUtil():
    # Chrome driver download: https://chromedriver.chromium.org/downloads
    from selenium import webdriver
    import time
    from selenium.webdriver.common.keys import Keys

    driver = webdriver.Chrome(executable_path='C:/Users/vhdlr/Desktop/mygithubrepo/pyProj/power2choose/chromedriver_win32/chromedriver.exe')
    driver.get('http://powertochoose.org')

    #homezipcode
    field_id = 'homezipcode'
    field = driver.find_element_by_id(field_id)
    field.send_keys('77494')
    field.send_keys(Keys.ENTER)
    field.submit()

    #view_all_results
    # time.sleep(5)
    # btn_id = 'view_all_results'
    # btn = driver.find_element_by_id(btn_id)
    # btn.click()

    #export-excel
    # xpath = '//*[@id="resultsForm"]/div/div/div[1]/div[1]/div[3]/a[2]'
    # btn_id = 'export-excel'
    # class_name = 'export-excel'
    # selector = '#resultsForm > div > div > div.paging-box.tc.paging-grid.k-pager-wrap.k-widget > div.sorting.lower-dropdown > div:nth-child(3) > a.export-excel'
    # selector = 'a.export-excel'
    # driver.current_url
    # btn = driver.find_element_by_class_name(class_name)
    # btn = driver.find_element_by_xpath(xpath)
    # btn = driver.find_element_by_link_text(btn_id)
    # btn = driver.find_element_by_css_selector(selector)
    # btn.click()
    # driver.get('http://powertochoose.org')
    driver.get('http://powertochoose.org/en-us/Plan/Results#')

    
    #resultsForm > div > div > div.paging-box.bc.paging-grid-footer.k-pager-wrap.k-widget > div.sorting > div:nth-child(3) > a.export-excel
    #resultsForm > div > div > div.paging-box.bc.paging-grid-footer.k-pager-wrap.k-widget > div.sorting > div:nth-child(3) > a.export-excel
    # //*[@id="resultsForm"]/div/div/div[1]/div[1]/div[3]/a[2]
    # http://powertochoose.org/en-us/Plan/Results#
    #resultsForm > div > div > div.paging-box.tc.paging-grid.k-pager-wrap.k-widget > div.sorting.lower-dropdown > div:nth-child(3) > a.export-excel
    #resultsForm > div > div > div.paging-box.tc.paging-grid.k-pager-wrap.k-widget > div.sorting.lower-dropdown > div:nth-child(3) > a.export-excel
    

    #homezipcode
    time.sleep(5) # Let the user actually see something!
    driver.quit()

    # driver = webdriver.Chrome('/path/to/chromedriver')  # Optional argument, if not specified will search path.
    # driver.get('http://www.google.com/');
    # time.sleep(5) # Let the user actually see something!
    # search_box = driver.find_element_by_name('q')
    # search_box.send_keys('ChromeDriver')
    # search_box.submit()
    # time.sleep(5) # Let the user actually see something!
    # driver.quit()
#----------------------------------------------------------------------

# if __name__ == "__main__":
def main():
    path = "power-to-choose.xlsx"
    # open_file_example(path)

    book = xlrd.open_workbook(path)
    p2c_sheet = book.sheet_by_index(0)
    numRows = p2c_sheet.nrows
    numCols = p2c_sheet.ncols

    estimatedUsage = 2500

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

    # plotUsageHistory(pastUsageData)

    runWebUtil()



if __name__ == "__main__":
    main()