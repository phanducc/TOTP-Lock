function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Data");
  var timestamp = new Date();
  var user = e.parameter.user;
  var action = e.parameter.action;

  if (user && action) {
    sheet.appendRow([timestamp, user, action]);
    return ContentService.createTextOutput("Success");
  } else {
    return ContentService.createTextOutput("Missing parameters");
  }
}