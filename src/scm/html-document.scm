;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; html-document.scm : generate HTML programmatically, with support
;; for simple style elements. 
;; Copyright 2000 Bill Gribble <grib@gnumatic.com>
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(gnc:support "html-document.scm")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  <html-document> class 
;;  this is the top-level object representing an entire HTML document.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define <html-document> 
  (make-record-type "<html-document>" 
                    '(style-sheet style-stack style title objects)))

(define gnc:html-document? 
  (record-predicate <html-document>))

(define gnc:make-html-document-internal
  (record-constructor <html-document>))

(define (gnc:make-html-document)
  (gnc:make-html-document-internal 
   #f                    ;; the stylesheet 
   '()                   ;; style stack
   (gnc:make-html-style-table) ;; document style info
   ""                    ;; document title
   '()                   ;; subobjects 
   ))

(define gnc:html-document-set-title!
  (record-modifier <html-document> 'title))

(define gnc:html-document-title
  (record-accessor <html-document> 'title))

(define gnc:html-document-set-style-sheet!
  (record-modifier <html-document> 'style-sheet))

(define gnc:html-document-style-sheet
  (record-accessor <html-document> 'style-sheet))

(define gnc:html-document-set-style-stack!
  (record-modifier <html-document> 'style-stack))

(define gnc:html-document-style-stack
  (record-accessor <html-document> 'style-stack))

(define gnc:html-document-set-style-internal!
  (record-modifier <html-document> 'style))

(define gnc:html-document-style
  (record-accessor <html-document> 'style))

(define gnc:html-document-set-objects!
  (record-modifier <html-document> 'objects))

(define gnc:html-document-objects
  (record-accessor <html-document> 'objects))

(define gnc:html-document?
  (record-predicate <html-document>))

(define (gnc:html-document-set-style! doc tag . rest)
  (let ((newstyle #f))
    (if (and (= (length rest) 2)
             (procedure? (car rest)))
        (set! newstyle 
              (apply gnc:make-html-data-style-info rest))
        (set! newstyle 
              (apply gnc:make-html-markup-style-info rest)))
    (gnc:html-style-table-set! (gnc:html-document-style doc) tag newstyle)))

(define (gnc:html-document-tree-collapse tree)
  (let ((retval '()))
    (define (do-list list)
      (for-each
       (lambda (elt)
         (if (string? elt)
             (set! retval (cons elt retval))
             (if (not (list? elt))
                 (set! retval
                       (cons (with-output-to-string 
                               (lambda () (display elt)))
                             retval))
                 (do-list elt))))
       list))
    (do-list tree)
    retval))

(define (gnc:html-document-render doc . rest) 
  (let ((stylesheet (gnc:html-document-style-sheet doc))
        (headers? (if (null? rest) #f (if (car rest) #t #f))))
    (if stylesheet 
        ;; if there's a style sheet, let it do the rendering 
        (gnc:html-style-sheet-render stylesheet doc headers?)
        
        ;; otherwise, do the trivial render. 
        (let* ((retval '())
               (push (lambda (l) (set! retval (cons l retval)))))
          ;; compile the doc style 
          (gnc:html-style-table-compile (gnc:html-document-style doc)
                                        (gnc:html-document-style-stack doc))
          ;; push it 
          (gnc:html-document-push-style doc (gnc:html-document-style doc))

          (if (not (null? headers?))
              (begin 
                (push "<html>\n")
                (push "<head>\n")
                (let ((title (gnc:html-document-title doc)))
                  (if title 
                      (push (list "</title>" title "<title>\n"))))
                (push "</head>\n")
                
                ;; this lovely little number just makes sure that <body>
                ;; attributes like bgcolor get included 
                (push ((gnc:html-markup/no-end "body") doc))))
          
          ;; now render the children
          (for-each-in-order 
           (lambda (child) 
             (push (gnc:html-object-render child doc)))
           (gnc:html-document-objects doc))

          (if (not (null? headers?))
              (begin 
                (push "</body>\n")
                (push "</html>\n")))
          
          (gnc:html-document-pop-style doc)
          (gnc:html-style-table-uncompile (gnc:html-document-style doc))

          (apply string-append 
                 (gnc:html-document-tree-collapse retval))))))


(define (gnc:html-document-push-style doc style)
  (gnc:html-document-set-style-stack! 
   doc (cons style (gnc:html-document-style-stack doc))))

(define (gnc:html-document-pop-style doc)
  (if (not (null? (gnc:html-document-style-stack doc)))
      (gnc:html-document-set-style-stack! 
       doc (cdr (gnc:html-document-style-stack doc)))))

(define (gnc:html-document-add-object! doc obj)
  (gnc:html-document-set-objects! 
   doc
   (append (gnc:html-document-objects doc) 
           (list (gnc:make-html-object obj)))))

(define (gnc:html-document-append-objects! doc objects)
  (gnc:html-document-set-objects!
   doc
   (append (gnc:html-document-objects doc) objects)))

(define (gnc:html-document-fetch-markup-style doc markup)
  (let ((style-info #f)
	(style-stack (gnc:html-document-style-stack doc)))
    (if (not (null? style-stack))
        (set! style-info 
              (gnc:html-style-table-fetch 
               (car style-stack)
               (cdr style-stack)
               markup)))
    (if (not style-info)
        (gnc:make-html-markup-style-info)
        style-info)))

(define (gnc:html-document-fetch-data-style doc markup)
  (let ((style-info #f)
	(style-stack (gnc:html-document-style-stack doc)))
    (if (not (null? (gnc:html-document-style-stack doc)))
        (set! style-info 
              (gnc:html-style-table-fetch 
               (car style-stack)
               (cdr style-stack)
               markup)))
    (if (not style-info)
        (gnc:make-html-data-style-info
         (lambda (datum parms)
           (sprintf #f "%a %a" markup datum))
         #f)
        style-info)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  markup-rendering functions : markup-start and markup-end return
;;  pre-body and post-body HTML for the given markup tag.
;;  the optional rest arguments are lists of attribute-value pairs:
;;  (gnc:html-document-markup-start doc "markup" 
;;                                 '("attr1" "value1") '("attr2" "value2"))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (gnc:html-document-markup-start doc markup . rest)
  (let ((childinfo (gnc:html-document-fetch-markup-style doc markup))
        (extra-attrib
         (if (not (null? rest))
             rest #f))
        (show-result #f))
    ;; now generate the start tag
    (let ((tag   (gnc:html-markup-style-info-tag childinfo))
          (attr  (gnc:html-markup-style-info-attributes childinfo))
          (face  (gnc:html-markup-style-info-font-face childinfo))
          (size  (gnc:html-markup-style-info-font-size childinfo))
          (color (gnc:html-markup-style-info-font-color childinfo)))

      ;; "" tags mean "show no tag"; #f tags means use default.
      (cond ((not tag)
             (set! tag markup))
            ((and (string? tag) (string=? tag ""))
             (set! tag #f)))
      (let* ((retval '())
             (push (lambda (l) (set! retval (cons l retval))))
             (add-internal-tag (lambda (tag) (push "<") (push tag) (push ">")))
             (add-attribute
              (lambda (key value prior) (push " ") (push key) 
		      (if value (begin (push "=")
				       (push value)))
		      #t))
             (addextraatt
              (lambda (attr)
                (cond ((string? attr) (push " ") (push attr))
                      (attr (gnc:warn "non-string attribute" attr)))))
             (build-first-tag
              (lambda (tag)
                (push "\n<") (push tag)
                (if attr (hash-fold add-attribute #f attr))
                (if extra-attrib (for-each addextraatt extra-attrib))
                (push ">"))))
        (if tag
            (if (list? tag)
                (begin
                  (build-first-tag (car tag))
                  (for-each add-internal-tag (cdr tag)))
                (build-first-tag tag)))
        (if (or face size color)
            (begin
              (push "<font ")
              (if face
                  (begin
                    (push "face=\"") (push face) (push "\" ")))
              (if size
                  (begin
                    (push "size=\"") (push size) (push "\" ")))
              (if color
                  (begin
                    (push "color=\"") (push color) (push "\" ")))
              (push ">")))
        retval))))

(define (gnc:html-document-markup-end doc markup)
  (let ((childinfo (gnc:html-document-fetch-markup-style doc markup)))
    ;; now generate the end tag
    (let ((tag (gnc:html-markup-style-info-tag childinfo))
          (closing-font-tag
           (gnc:html-markup-style-info-closing-font-tag childinfo)))
      ;; "" tags mean "show no tag"; #f tags means use default.
      (cond ((not tag)
             (set! tag markup))
            ((and (string? tag) (string=? tag ""))
             (set! tag #f)))
      (let* ((retval '())
             (push (lambda (l) (set! retval (cons l retval)))))
        (if closing-font-tag
            (push "</font>\n"))
        (if tag
            (let ((addtag (lambda (t)
                            (push "</")
                            (push tag)
                            (push ">\n"))))
              (cond
               ((string? tag)
                (addtag tag))
               ((list? tag)
                (for-each addtag (reverse tag))))))
        retval))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  html-document-render-data 
;;  looks up the relevant data style and renders the data accordingly
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (gnc:html-document-render-data doc data)
  (let ((style-info #f)
        (data-type #f))
    (cond 
     ((number? data)
      (set! data-type "<number>"))
     ((string? data)
      (set! data-type "<string>"))
     ((boolean? data)
      (set! data-type "<boolean>"))
     ((record? data)
      (set! data-type (record-type-name (record-type-descriptor data))))
     (#t 
      (set! data-type "<generic>")))
    
    (set! style-info (gnc:html-document-fetch-data-style doc data-type))
    
    ((gnc:html-data-style-info-renderer style-info)
     data (gnc:html-data-style-info-data style-info))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;  <html-object> class
;;  this is the parent of all the html object types.  You should not
;;  be creating <html-object> directly... use the specific type you
;;  want.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define <html-object>
  (make-record-type "<html-object>"
                    '(renderer data)))
(define gnc:html-object? 
  (record-predicate <html-object>))

(define gnc:make-html-object-internal
  (record-constructor <html-object>))

(define (gnc:make-html-object obj)
  (let ((o #f))
    (if (not (record? obj))
        ;; for literals (strings/numbers)
        (set! o
              (gnc:make-html-object-internal 
               (lambda (obj doc)
                 (gnc:html-document-render-data doc obj))
               ;; if the object is #f, make it a placeholder
               (if obj obj "&nbsp;&nbsp;&nbsp;")))        
        (cond 
         ((gnc:html-text? obj)
          (set! o (gnc:make-html-object-internal 
                   gnc:html-text-render obj)))
         ((gnc:html-table? obj)
          (set! o (gnc:make-html-object-internal 
                   gnc:html-table-render obj)))
         ((gnc:html-table-cell? obj)
          (set! o (gnc:make-html-object-internal 
                   gnc:html-table-cell-render obj)))
         ((gnc:html-barchart? obj)
          (set! o (gnc:make-html-object-internal 
                   gnc:html-barchart-render obj)))
         ((gnc:html-piechart? obj)
          (set! o (gnc:make-html-object-internal 
                   gnc:html-piechart-render obj)))
         ((gnc:html-scatter? obj)
          (set! o (gnc:make-html-object-internal 
                   gnc:html-scatter-render obj)))
         ((gnc:html-object? obj)
          (set! o obj))
         
         ;; other record types that aren't HTML objects 
         (#t 
          (set! o
                (gnc:make-html-object-internal 
                 (lambda (obj doc)
                   (gnc:html-document-render-data doc obj))
                 obj)))))
    o))

(define gnc:html-object-renderer
  (record-accessor <html-object> 'renderer))

(define gnc:html-object-set-renderer!
  (record-modifier <html-object> 'renderer))

(define gnc:html-object-data
  (record-accessor <html-object> 'data))

(define gnc:html-object-set-data!
  (record-modifier <html-object> 'data))

(define (gnc:html-object-render obj doc)
  (if (gnc:html-object? obj) 
      ((gnc:html-object-renderer obj) (gnc:html-object-data obj) doc)
      (let ((htmlo (gnc:make-html-object obj)))
        (gnc:html-object-render htmlo doc))))