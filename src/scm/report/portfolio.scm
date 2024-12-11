;; -*-scheme-*- ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; portfolio.scm
;; by Robert Merkel (rgmerk@mira.net)
;;
;; This program is free software; you can redistribute it and/or    
;; modify it under the terms of the GNU General Public License as   
;; published by the Free Software Foundation; either version 2 of   
;; the License, or (at your option) any later version.              
;;                                                                  
;; This program is distributed in the hope that it will be useful,  
;; but WITHOUT ANY WARRANTY; without even the implied warranty of   
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    
;; GNU General Public License for more details.                     
;;                                                                  
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, contact:
;;
;; Free Software Foundation           Voice:  +1-617-542-5942
;; 59 Temple Place - Suite 330        Fax:    +1-617-542-2652
;; Boston, MA  02111-1307,  USA       gnu@gnu.org
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; depends must be outside module scope -- and should eventually go away.
(gnc:depend  "report-html.scm")

(define-module (gnucash report portfolio))

(use-modules (srfi srfi-1))
(use-modules (ice-9 slib))
(require 'printf)

(define optname-price-source (N_ "Price Source"))

(define (options-generator)
  (let* ((options (gnc:new-options)) 
         ;; This is just a helper function for making options.
         ;; See gnucash/src/scm/options.scm for details.
         (add-option 
          (lambda (new-option)
            (gnc:register-option options new-option))))

    ;; General Tab
    ;; date at which to report balance
    (gnc:options-add-report-date!
     options gnc:pagename-general 
     (N_ "Date") "a")

    (gnc:options-add-currency! 
     options gnc:pagename-general (N_ "Report Currency") "c")

    (gnc:options-add-price-source! 
     options gnc:pagename-general
     optname-price-source "d" 'pricedb-latest)

    ;; Account tab
    (add-option
     (gnc:make-account-list-option
      gnc:pagename-accounts (N_ "Accounts")
      "b"
      (N_ "Stock Accounts to report on")
      (lambda () (filter gnc:account-is-stock?
                         (gnc:group-get-subaccounts
                          (gnc:get-current-group))))
      (lambda (accounts) (list  #t 
                                (filter gnc:account-is-stock? accounts)))
      #t))

    (gnc:options-set-default-section options gnc:pagename-general)      
    options))

;; This is the rendering function. It accepts a database of options
;; and generates an object of type <html-document>.  See the file
;; report-html.txt for documentation; the file report-html.scm
;; includes all the relevant Scheme code. The option database passed
;; to the function is one created by the options-generator function
;; defined above.
(define (portfolio-renderer report-obj)
  
  ;; These are some helper functions for looking up option values.
  (define (get-op section name)
    (gnc:lookup-option (gnc:report-options report-obj) section name))
  
  (define (get-option section name)
    (gnc:option-value (get-op section name)))
  
  (define (table-add-stock-rows table accounts to-date
                                currency price-fn collector)

    (define (table-add-stock-rows-internal accounts odd-row?)
      (if (null? accounts) collector
          (let* ((row-style (if odd-row? "normal-row" "alternate-row"))
                 (current (car accounts))
                 (rest (cdr accounts))
                 (name (gnc:account-get-name current))
                 (commodity (gnc:account-get-commodity current))
                 (ticker-symbol (gnc:commodity-get-mnemonic commodity))
                 (listing (gnc:commodity-get-namespace commodity))
                 (unit-collector (gnc:account-get-comm-balance-at-date
                                  current to-date #f))
                 (units (cadr (unit-collector 'getpair commodity #f)))

                 (price-value (price-fn commodity currency to-date))
                 
                 (value-num (gnc:numeric-mul
                             units 
                             price-value
                             (gnc:commodity-get-fraction currency)
                             GNC-RND-ROUND))

                 (value (gnc:make-gnc-monetary currency value-num)))
            (collector 'add currency value-num)
            (gnc:html-table-append-row/markup!
             table
             row-style
             (list name
                   ticker-symbol
                   listing
                   (gnc:make-html-table-header-cell/markup
                    "number-cell" (gnc:numeric-to-double units))
                   (gnc:make-html-table-header-cell/markup
                    "number-cell" (gnc:make-gnc-monetary currency
                                                         price-value))
                   (gnc:make-html-table-header-cell/markup
                    "number-cell" value)))
            (table-add-stock-rows-internal rest (not odd-row?)))))

    (table-add-stock-rows-internal accounts #t))

  ;; The first thing we do is make local variables for all the specific
  ;; options in the set of options given to the function. This set will
  ;; be generated by the options generator above.
  (let ((to-date     (gnc:date-option-absolute-time
                      (get-option gnc:pagename-general "Date")))
        (accounts    (get-option gnc:pagename-accounts "Accounts"))
        (currency    (get-option gnc:pagename-general "Report Currency"))
        (report-title (get-option gnc:pagename-general 
                                  gnc:optname-reportname))
        (price-source (get-option gnc:pagename-general
                                  optname-price-source))

        (collector   (gnc:make-commodity-collector))
        ;; document will be the HTML document that we return.
        (table (gnc:make-html-table))
        (document (gnc:make-html-document)))

    (gnc:html-document-set-title!
     document (string-append 
               report-title
               (sprintf #f " %s" (gnc:timepair-to-datestring to-date))))

    (gnc:debug "accounts" accounts)
    (if (not (null? accounts))
        (let* ((commodity-list (gnc:accounts-get-commodities 
                                (append 
                                 (gnc:acccounts-get-all-subaccounts 
                                  accounts) accounts) currency))
               (pricedb (gnc:book-get-pricedb (gnc:get-current-book)))
               (price-fn
                (case price-source
                  ('weighted-average 
                   (let ((pricealist 
                          (gnc:get-commoditylist-totalavg-prices
                           commodity-list currency to-date)))
                     (lambda (foreign domestic date) 
                       (gnc:pricealist-lookup-nearest-in-time
                        pricealist foreign date))))
                  ('pricedb-latest 
                   (lambda (foreign domestic date) 
                     (let ((price
                            (gnc:pricedb-lookup-latest
                             pricedb foreign domestic)))
                       (if price
                           (let ((v (gnc:price-get-value price)))
                             (gnc:price-unref price)
                             v)
                           (gnc:numeric-zero)))))
                  ('pricedb-nearest 
                   (lambda (foreign domestic date) 
                     (let ((price
                            (gnc:pricedb-lookup-nearest-in-time 
                             pricedb foreign domestic date)))
                       (if price
                           (let ((v (gnc:price-get-value price)))
                             (gnc:price-unref price)
                             v)
                           (gnc:numeric-zero))))))))
          
          (gnc:html-table-set-col-headers!
           table
           (list (_ "Account")
                 (_ "Symbol")
                 (_ "Listing")
                 (_ "Units")
                 (_ "Price")
                 (_ "Value")))
          
          (table-add-stock-rows
           table accounts to-date currency 
           price-fn collector)
          
          (gnc:html-table-append-row/markup!
           table
           "grand-total"
           (list
            (gnc:make-html-table-cell/size
             1 6 (gnc:make-html-text (gnc:html-markup-hr)))))
          
          (collector
           'format 
           (lambda (currency amount)
             (gnc:html-table-append-row/markup! 
              table
              "grand-total"
              (list (gnc:make-html-table-cell/markup
                     "total-label-cell" (_ "Total"))
                    (gnc:make-html-table-cell/size/markup
                     1 5 "total-number-cell"
                     (gnc:make-gnc-monetary currency amount)))))
           #f)
          
          (gnc:html-document-add-object! document table))

                                        ;if no accounts selected.
        (gnc:html-document-add-object!
         document 
	 (gnc:html-make-no-account-warning 
	  report-title (gnc:report-id report-obj))))
    
    document))

(gnc:define-report
 'version 1
 'name (N_ "Investment Portfolio")
 'menu-path (list gnc:menuname-asset-liability)
 'options-generator options-generator
 'renderer portfolio-renderer)