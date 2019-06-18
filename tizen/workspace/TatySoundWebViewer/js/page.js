/*
 *      Copyright (c) 2016 Samsung Electronics Co., Ltd
 *
 *      Licensed under the Flora License, Version 1.1 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *              http://floralicense.org/license/
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

/*exported pageController*/

var pageController = (function() {
    var self = this,
        pageNow = "",
        pageNowIndex = -1,
        pageMain = "",
        pageMainIndex = -1,
        pageList = [],
        pageHistory = [];

    /**
     * Pushes a page to the history list.
     * @private
     * @param {string} page Name of the page to be pushed.
     */
    function pushHistory(page) {
        pageHistory.push(page);
    }

    /**
     * Pops a page from the history list.
     * @private
     * @return {string} name of the popped page.
     */
    function popHistory() {
        var retPage;

        if (pageHistory.length > 0) {
            retPage = pageHistory.pop();
        } else {
            console.warn("ERROR: Failed to popHistory - PageHistory is Empty");

            return null;
        }

        return retPage;
    }

    /**
     * Shows selected page and hide all other pages.
     * @private
     * @param {string} page Name of the page to be displayed.
     * @return {boolean} true if the page is successfully displayed.
     */
    function showPage(page) {
        var i,
            destPage = document.querySelector("#" + page),
            objPage;

        if (destPage !== null) {
            for (i = 0; i < pageList.length; i++) {
                objPage = document.querySelector("#" + pageList[i]);
                objPage.style.display = "none";
            }

            destPage.style.display = "block";
            pageNow = page;
        } else {
            console.warn("ERROR: Page named " + page + " is not exist.");

            return false;
        }

        return true;
    }

    /**
     * Adds a new page to the page list.
     * @public
     * @param {string} page The name of a page to be added.
     * @param {number} index The index of a page to be added.
     * @return {boolean} true if the page is successfully added.
     */
    this.addPage = function(page, index) {
        var objPage = document.querySelector("#" + page);

        if (objPage) {
            if (index) {
                pageList.splice(index, 0, page);
            } else {
                pageList.push(page);
                index = pageList.length;
            }
        } else {
            console.warn("ERROR: Failed to addPage - The page doesn't exist");

            return false;
        }

        if (pageList.length === 1) {
            self.movePage(page) ;
        }

        if (pageMain === "") {
            pageMain = page;
            pageMainIndex = index;
        }

        return true;
    };

    /**
     * Removes a page from the page list.
     * @public
     * @param {string} page The name of the a page to be removed.
     * @return {boolean} true if the page is successfully removed.
     */
    this.removePage = function(page) {
        var pageIndex = pageList.indexOf(page);

        if (pageIndex > -1) {
            if (pageIndex === pageMainIndex) {
                pageMain = "";
                pageMainIndex = -1;
            }

            pageList.splice(pageIndex, 1);
        } else {
            console.warn("ERROR: Failed to removePage - The page doesn't exist in pageList");

            return false;
        }

        return true;
    };

    /**
     * Moves to the selected page.
     * @public
     * @param {string} page The name of a page to be displayed.
     * @return {boolean} true if the page is successfully displayed.
     */
    this.movePage = function(dest) {
        var lastPage = pageNow;

        if (showPage(dest)) {
            pushHistory(lastPage);
            pageNow = dest;
            pageNowIndex = pageList.indexOf(pageNow);
        } else {
            return false;
        }

        return true;
    };

    /**
     * Moves back to the last page of the history list.
     * @public
     * @return {boolean} true if the page is successfully displayed.
     */
    this.moveBackPage = function() {
        var beforePage = popHistory();

        if (beforePage !== null) {
            showPage(beforePage);
            pageNow = beforePage;
            pageNowIndex = pageList.indexOf(pageNow);
        } else {
            console.warn("ERROR: Failed to backPage - popHistory returned null");

            return false;
        }

        return true;
    };

    /**
     * Moves to the previous page of the page list.
     * @public
     * @return {boolean} true if the page is successfully displayed.
     */
    this.movePrevPage = function() {
        if (pageNowIndex > 0) {
            self.movePage(pageList[pageNowIndex - 1]);
        } else {
            console.warn("ERROR: Failed to movePrevPage - There is no previous page");

            return false;
        }

        return true;
    };

    /**
     * Moves to the next page of the page list.
     * @public
     * @return {boolean} true if the page is successfully displayed.
     */
    this.moveNextPage = function() {
        if (pageNowIndex < pageList.length - 1) {
            self.movePage(pageList[pageNowIndex + 1]);
        } else {
            console.warn("ERROR: Failed to moveNextPage - There is no next page");

            return false;
        }

        return true;
    };

    /**
     * Gets the name of the current page.
     * @public
     * @return {string} The name of the current page.
     */
    this.getPageNow = function() {
        return pageNow;
    };

    /**
     * Returns the boolean value whether the current page is the main page or not.
     * @public
     * @return {boolean} true if the current page is the main page.
     */
    this.isPageMain = function() {
        return (pageNow === pageMain);
    };

    /**
     * Returns the boolean value whether the page is already added to the page list or not.
     * @public
     * @return {boolean} true if the page is already added.
     */
    this.isPageExist = function(page) {
        return (pageList.indexOf(page) > -1);
    };

    /**
     * Sets the background image of the page.
     * @param {string} page - Name of the page to be set the background image.
     * @param {string} imagePath - Path of the background image.
     * @public
     */
    this.setPageBgImage = function(page, imagePath) {
        var elmPage;

        if (self.isPageExist(page) === true) {
            elmPage = document.querySelector("#" + page);
            if (imagePath && typeof imagePath === "string") {
                elmPage.style.backgroundImage = "url(" + imagePath + ")";
            } else {
                elmPage.style.backgroundImage = "";
            }
        }
    };

    return this;
}());
