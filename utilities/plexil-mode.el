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

;;; Example installation instructions (variations are possible):
;;;    1. In your home directory, place this in your .emacs file.
;;;
;;;           (add-to-list 'load-path "~/.emacs.d/lisp")
;;;           (load "plexil-mode")
;;;           (add-to-list 'auto-mode-alist '("\\.ple\\'" . plexil-mode))
;;;           (add-to-list 'auto-mode-alist '("\\.plp\\'" . plexil-mode))
;;;
;;;    2. Place this file (plexil-mode.el) in your ~/.emacs.d/lisp directory.


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
				   "for" "Priority") 'words))

(setq constantsRegexp (regexp-opt '("true" "false" "Unknown") 'words))

;; set highlights
(setq plexilHighlights
      `(("[a-z0-9A-Z_]+:\\|time" . font-lock-type-face)
	("Integer\\|Boolean\\|Real\\|String\\|Date" . font-lock-type-face)
	(,keywordsRegexp . font-lock-keyword-face)
	(,builtinRegexp . font-lock-builtin-face)
	(,constantsRegexp . font-lock-constant-face)
	("Comment" . font-lock-comment-face)
	(,functionsRegexp . font-lock-function-name-face)
	))


(defcustom plexil-mode-common-hook nil
  "Normal hook run when entering 'plexil-mode'.
See 'run-hooks'."
  :type 'hook
  :group 'plexil
  )

;; Correct some indentation issues with c-indent-line-or-region.
(defun set-newline-and-indent-node ()

  (if (looking-at "[ \t]*[a-z0-9A-Z_]+[:]?[ ]+[a-z0-9A-Z]*")
      (progn (local-set-key (kbd ":") '(":"))
	     (local-set-key (kbd "{") '("{"))
	     (local-set-key (kbd "}") '("}"))
	     (local-set-key (kbd ")") '(")"))
	     (local-set-key (kbd "(") '("("))
	     (local-set-key (kbd ";") '(";"))
	     ))
  (if (looking-at "[...]*{")
      ;;(local-set-key (kbd TAB) '(DEL))
    (local-set-key (kbd "RET") '("RET"))
    (local-set-key (kbd ":") '(":"))
    (local-set-key (kbd "{") '("{")))

  ;; Supposed to stop TAB from indenting on an empty line in a new node. Not currently working.
  (if (looking-at "[...]*{[ \t]*[\n]+")
      (local-set-key (kbd TAB) '(nil)))

  )

(add-hook 'plexil-mode-hook 'set-newline-and-indent-node)


;; Set C++ style to a more appropriate indentation style for PLEXIL
;; and a proper offset.
(add-hook 'plexil-mode-hook
	  (setq c-default-style "ellemtel")
          (setq	c-basic-offset 2)
	  )

;; Define mode, derive from c++-mode. Set syntax highlighting and indentation.
(define-derived-mode plexil-mode c++-mode "PLEXIL"
  :syntax-table plexil-mode-syntax-table
  (setq font-lock-defaults '(plexilHighlights))
  (font-lock-fontify-buffer)
  (run-mode-hooks 'plexil-mode-hook)
  )
