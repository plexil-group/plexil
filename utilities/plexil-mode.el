;;; Copyright (c) 2006-2020, Universities Space Research Association (USRA).
;;;  All rights reserved.
;;;
;;; Redistribution and use in source and binary forms, with or without
;;; modification, are permitted provided that the following conditions are met:
;;;     * Redistributions of source code must retain the above copyright
;;;       notice, this list of conditions and the following disclaimer.
;;;     * Redistributions in binary form must reproduce the above copyright
;;;       notice, this list of conditions and the following disclaimer in the
;;;       documentation and/or other materials provided with the distribution.
;;;     * Neither the name of the Universities Space Research Association nor the
;;;       names of its contributors may be used to endorse or promote products
;;;       derived from this software without specific prior written permission.
;;;
;;; THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
;;; WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
;;; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;;; DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
;;; INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
;;; BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
;;; OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
;;; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
;;; TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
;;; USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


;;; PLEXIL -- A Major Mode for PLEXIL.

;;; Basic instructions:
;;;    1. In your home/user directory, place this in your .emacs file.
;;;
;;;           (add-to-list 'load-path "~/.emacs.d/lisp/")
;;;           (load "plexil-mode")
;;;           (add-to-list 'auto-mode-alist '("\\.ple\\'" . plexil-mode))
;;;
;;;    2. Place this file (plexil-mode.el) in your home/user/.emacs.d/lisp
;;;       directory.
;;;
;;;
;;; NOTE: Some c-mode functions are in this file without modification. These
;;; are here to be modified in order to correct some indentation problems in
;;; the future.



(defconst plexil-mode-syntax-table
  (let ((table (make-syntax-table)))
    ;; " is a string quote character.
    (modify-syntax-entry ?\" "\"" table)

    ;; // starts a single-line comment, / is punctuation.
    (modify-syntax-entry ?/ ". 12" table)
    ;; /n ends a single-line comment.
    (modify-syntax-entry ?\n ">" table)
    table)
  )

;; Set keywords
(setq builtinRegexp (regexp-opt '("Command" "Lookup" "StartCondition" "EndCondition"
				  "RepeatCondition" "SkipCondition" "PreCondition"
				  "PostCondition" "InvariantCondtion" "Concurrence"
				  "UncheckedSequence" "Sequence" "LibraryCall"
				  "LibraryAction" "Update" "Try" "OnCommand"
				  "OnMessage" "SynchronousCommand" "Wait"
				  "Start" "Exit" "Repeat" "Comment" "Pre"
				  "Post" "Repeat" "Repeat-while") 'words))

(setq functionsRegexp (regexp-opt '( "sqrt" "abs" "ceil" "floor" "round" "trunc"
				     "real_to_int" "pprint" "print") 'words))

(setq keywordsRegexp (regexp-opt '("InOut" "In" "if" "else" "elseif" "endif" "while"
				   "for") 'words))

(setq constantsRegexp (regexp-opt '("true" "false" "Unknown") 'words))

;; set highlights
(setq plexilHighlights
      `(("[a-z0-9A-Z_]+:\\|time" . font-lock-type-face)
	("Integer\\|Boolean\\|Real\\|String\\|Date" . font-lock-type-face)
	(,keywordsRegexp . font-lock-keyword-face)
	(,builtinRegexp . font-lock-builtin-face)
	(,constantsRegexp . font-lock-constant-face)
	("Comment" . font-lock-comment-face)
	))


(defcustom plexil-mode-hook nil
  "Normal hook run when entering 'plexil-mode'.
See 'run-hooks'."
  :type 'hook
  :group 'plexil
  )


;; Correct some indentation issues with c-indent-line-or-region.
(defun set-newline-and-indent-node ()
  (while (looking-at "[ \t]*[a-z0-9A-Z_]+[:]?[ ]?[a-z0-9A-Z]*")
    (progn (local-set-key (kbd ":") '(":"))
	   (local-set-key (kbd "{") '("{"))
	   (local-set-key (kbd "}") '("}"))
	   (local-set-key (kbd ")") '(")"))
	   (local-set-key (kbd "(") '("("))
	   (local-set-key (kbd ";") '(";"))
	   (local-set-key (kbd RET) '(progn(newline)
					   (c-electric-backspace)
					   (c-electric-backspace)))
	   ))
  (if (looking-at "[...]*{")
      (local-set-key (kbd TAB) '(DEL))
    (local-set-key (kbd "RET") '(progn '(newline-and-indent))
		   ))
  )

(add-hook 'c-mode-common-hook 'set-newline-and-indent-node)

(defun c-indent-command (&optional arg)
  "Indent current line as C code, and/or insert some whitespace.
If `c-tab-always-indent' is t, always just indent the current line.
If nil, indent the current line only if point is at the left margin or
in the line's indentation; otherwise insert some whitespace[*].  If
other than nil or t, then some whitespace[*] is inserted only within
literals (comments and strings), but the line is always reindented.
If `c-syntactic-indentation' is t, indentation is done according to
the syntactic context.  A numeric argument, regardless of its value,
means indent rigidly all the lines of the expression starting after
point so that this line becomes properly indented.  The relative
indentation among the lines of the expression is preserved.
If `c-syntactic-indentation' is nil, the line is just indented one
step according to `c-basic-offset'.  In this mode, a numeric argument
indents a number of such steps, positive or negative, and an empty
prefix argument is equivalent to -1.
  [*] The amount and kind of whitespace inserted is controlled by the
  variable `c-insert-tab-function', which is called to do the actual
  insertion of whitespace.  Normally the function in this variable
  just inserts a tab character, or the equivalent number of spaces,
  depending on the variable `indent-tabs-mode'."

  (interactive "P")
  (let ((indent-function
	 (if c-syntactic-indentation
	     (symbol-function 'indent-according-to-mode)
	   (lambda ()
	     (let ((c-macro-start c-macro-start)
		   (steps (if (equal arg '(4))
			      -1
			    (prefix-numeric-value arg))))
	       (c-shift-line-indentation (* steps c-basic-offset))
	       (when (and c-auto-align-backslashes
			  (save-excursion
			    (end-of-line)
			    (eq (char-before) ?\\))
			  (c-query-and-set-macro-start))
		 ;; Realign the line continuation backslash if inside a macro.
		 (c-backslash-region (point) (point) nil t)))
	     ))))
    (if (and c-syntactic-indentation arg)
	;; If c-syntactic-indentation and got arg, always indent this
	;; line as C and shift remaining lines of expression the same
	;; amount.
	(let ((shift-amt (save-excursion
			   (back-to-indentation)
			   (current-column)))
	      beg end)
	  (c-indent-line)
	  (setq shift-amt (- (save-excursion
			       (back-to-indentation)
			       (current-column))
			     shift-amt))
	  (save-excursion
	    (if (eq c-tab-always-indent t)
		(beginning-of-line))	; FIXME!!! What is this here for?  ACM 2005/10/31
	    (setq beg (point))
	    (c-forward-sexp 1)
	    (setq end (point))
	    (goto-char beg)
	    (forward-line 1)
	    (setq beg (point)))
	  (if (> end beg)
	      (indent-code-rigidly beg end shift-amt "#")))
      ;; Else use c-tab-always-indent to determine behavior.
      (cond
       ;; CASE 1: indent when at column zero or in line's indentation,
       ;; otherwise insert a tab
       ((not c-tab-always-indent)
	(if (save-excursion
	      (skip-chars-backward " \t")
	      (not (bolp)))
	    (funcall c-insert-tab-function)
	  (funcall indent-function)))
       ;; CASE 2: just indent the line
       ((eq c-tab-always-indent t)
	(funcall indent-function))
       ;; CASE 3: if in a literal, insert a tab, but always indent the
       ;; line
       (t
	(if (c-save-buffer-state () (c-in-literal))
	    (funcall c-insert-tab-function))
	(funcall indent-function)
	)))))

(defun c-indent-region (start end &optional quiet)
  "Indent syntactically every line whose first char is between START
and END inclusive.  If the optional argument QUIET is non-nil then no
syntactic errors are reported, even if `c-report-syntactic-errors' is
non-nil."
  (save-excursion
    (goto-char end)
    (skip-chars-backward " \t\n\r\f\v")
    (setq end (point))
    (goto-char start)
    ;; Advance to first nonblank line.
    (beginning-of-line)
    (skip-chars-forward " \t\n\r\f\v")
    (setq start (point))
    (beginning-of-line)
    (setq c-parsing-error
	  (or (let ((endmark (copy-marker end))
		    (c-parsing-error nil)
		    ;; shut up any echo msgs on indiv lines
		    (c-echo-syntactic-information-p nil)
		    (ml-macro-start	; Start pos of multi-line macro.
		     (and (c-save-buffer-state ()
					       (save-excursion (c-beginning-of-macro)))
			  (eq (char-before (c-point 'eol)) ?\\)
			  start))
		    (c-fix-backslashes nil)
		    syntax)
		(unwind-protect
		    (progn
		      (c-progress-init start end 'c-indent-region)

		      (while (and (bolp) ;; One line each time round the loop.
				  (not (eobp))
				  (< (point) endmark))
			;; update progress
			(c-progress-update)
			;; skip empty lines
			(unless (or (looking-at "\\s *$")
				    (and ml-macro-start (looking-at "\\s *\\\\$")))
			  ;; Get syntax and indent.
			  (c-save-buffer-state nil
					       (setq syntax (c-guess-basic-syntax)))
			  (c-indent-line syntax t t))

			(if ml-macro-start
			    ;; End of current multi-line macro?
			    (when (and c-auto-align-backslashes
				       (not (eq (char-before (c-point 'eol)) ?\\)))
			      ;; Fixup macro backslashes.
			      (c-backslash-region ml-macro-start (c-point 'bonl) nil)
			      (setq ml-macro-start nil))
			  ;; New multi-line macro?
			  (if (and (assq 'cpp-macro syntax)
				   (eq (char-before (c-point 'eol)) ?\\))
			      (setq ml-macro-start (point))))

			(forward-line))

		      (if (and ml-macro-start c-auto-align-backslashes)
			  (c-backslash-region ml-macro-start (c-point 'bopl) nil t)))
		  (set-marker endmark nil)
		  (c-progress-fini 'c-indent-region))
		(c-echo-parsing-error quiet))
	      c-parsing-error))))

(defun c-indent-line-or-region (&optional arg region)
  "Indent active region, current line, or block starting on this line.
In Transient Mark mode, when the region is active, reindent the region.
Otherwise, with a prefix argument, rigidly reindent the expression
starting on the current line.
Otherwise reindent just the current line."
  (interactive
   (list current-prefix-arg (c-region-is-active-p)))
  (if region
      (c-indent-region (region-beginning) (region-end))
    (c-indent-command arg)))

;; Define mode, derive from c++-mode. Set syntax highlighting and indentation.
(define-derived-mode plexil-mode c++-mode "PLEXIL"
  :syntax-table plexil-mode-syntax-table
  (setq font-lock-defaults '(plexilHighlights))
  (font-lock-fontify-buffer)
  (run-mode-hooks 'plexil-mode-hook)
  )

