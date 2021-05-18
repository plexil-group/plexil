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


;;; plexil-mode -- A Major Mode for PLEXIL, derived from c++-mode

;;; Example installation instructions (variations are possible):
;;;    1. In your home directory, place this in your .emacs file.
;;;
;;;           (add-to-list 'load-path "~/.emacs.d/lisp")
;;;           (load "plexil-mode")
;;;           (add-to-list 'auto-mode-alist '("\\.ple\\'" . plexil-mode))
;;;           (add-to-list 'auto-mode-alist '("\\.plp\\'" . plexil-mode))
;;;
;;;    2. Place this file (plexil-mode.el) in your ~/.emacs.d/lisp directory.

;;;
;;; Styled after Emacs-D-Mode:
;;;  https://github.com/Emacs-D-Mode-Maintainers/Emacs-D-Mode
;;;

;;;
;;; Required packages (largely from Emacs-D-Mode)
;;; 

(require 'cc-mode)
(require 'cc-langs)

;; Needed to prevent
;;   "Symbol's value as variable is void: compilation-error-regexp-alist-alist" errors
(require 'compile)

;; Work around Emacs (cc-mode) bug #18845
(eval-when-compile
  (when (and (= emacs-major-version 24) (>= emacs-minor-version 4))
    (require 'cl)))

;; These are only required at compile time to get the sources for the
;; language constants.  (The cc-fonts require and the font-lock
;; related constants could additionally be put inside an
;; (eval-after-load "font-lock" ...) but then some trickery is
;; necessary to get them compiled.)
;; Comment out 'when-compile part for debugging
(eval-when-compile
  (require 'cc-fonts))

;; (require 'derived) ; TODO: flush

;;;
;;; cc-mode configuration
;;;

(eval-and-compile
  ;; Make our mode known to the language constant system.
  ;; This needs to be done also at compile time since the language
  ;; constants are evaluated then.
  (c-add-language 'plexil-mode 'c-mode)) ; c++-mode? java-mode?

;;; Muffle the warnings about using undefined functions
(declare-function c-populate-syntax-table "cc-langs.el" (table))


;;;
;;; Syntax
;;;

;;; See cc-langs.el, d-mode.el

;;; Standard PLEXIL does not support pointers or references.
;;; The c-type-decl-prefix-key default should work.
;; (c-lang-defconst c-type-decl-prefix-key
;;   plexil
;;   nil)

;;; Primitive type keywords.  As opposed to the other keyword lists, the
;;; keywords listed here are fontified with the type face instead of the
;;; keyword face.
(c-lang-defconst c-primitive-type-kwds
  plexil
  '("Any" "Boolean" "Date" "Duration" "Integer" "Real" "String"))

;;; Keywords where the following name - if any - is a type name, and
;;; where the keyword together with the symbol works as a type in
;;; declarations.
;;; (This is not strictly true - In and InOut can prefix
;;; variable names inherited from a containing context.)
(c-lang-defconst c-type-prefix-kwds
  plexil
  '("In" "InOut"))

;;; Prefix keyword(s) like "typedef" which make a type declaration out
;;; of a variable declaration.
(c-lang-defconst c-typedef-kwds
  plexil
  nil)

;;; Keywords which return a value to the calling function.
(c-lang-defconst c-return-kwds
  plexil
  nil)

;;; Regexp matching the start of any declaration, cast or label.
(c-lang-defconst c-decl-start-re
  plexil
  (c-make-keywords-re nil
    (append (c-lang-const c-primitive-type-kwds)
            (c-lang-const c-type-prefix-kwds)
            (c-lang-const c-typeless-decl-kwds))
    'plexil))


;;; Keywords introducing declarations where the (first) identifier
;;; (declarator) follows directly after the keyword, without any type.
;;; (Does Lookup belong on this list?)
(c-lang-defconst c-typeless-decl-kwds
  plexil
  '("Command"
    "LibraryAction"
    ))

;;; Statement keywords followed directly by a substatement.
(c-lang-defconst c-block-stmt-1-kwds
  plexil
  '("CheckedSequence"
    "Concurrence"
    "Sequence"
    "Try"
    "UncheckedSequence"
    "do"
    "else"
    ))

;;; Statement keywords optionally followed by a paren sexp.
;;; Keywords here should also be in `c-block-stmt-1-kwds'.
(c-lang-defconst c-mode-block-stmt-1-2-kwds
  plexil
  nil)

;;; Statement keywords followed by a paren sexp and then by a substatement.
(c-lang-defconst c-block-stmt-2-kwds
  plexil
  '("OnCommand" "OnMessage" "elseif" "for" "if" "while"))

;;; Statement keywords followed by an expression or nothing.
(c-lang-defconst c-simple-stmt-kwds
  plexil
  '("Comment"
    "End"       "EndCondition"
    "Exit"      "ExitCondition"
    "Invariant" "InvariantCondition"
    "Post"      "PostCondition"
    "Pre"       "PreCondition"
    "Priority"
    "Repeat"    "RepeatCondition"
    "Resource"
    "Skip"      "SkipCondition"
    "Start"     "StartCondition"
    "Update"
    ))

;;; Keywords that may be followed by a parenthesis expression that doesn't
;;; contain type identifiers.
(c-lang-defconst c-paren-nontype-kwds
  plexil
  '(
    ;; Node refs
    "Child"
    "Sibling"

    ;; Built-in library functions
    ;; "NoChildFailed"
    ;; "NodeExecuting"
    ;; "NodeFailed"
    ;; "NodeFinished"
    ;; "NodeInactive"
    ;; "NodeInvariantFailed"
    ;; "NodeIterationEnded"
    ;; "NodeIterationFailed"
    ;; "NodeIterationSucceded"
    ;; "NodeParentFailed"
    ;; "NodePostconditionFailed"
    ;; "NodePreconditionFailed"
    ;; "NodeSkipped"
    ;; "NodeSucceeded"
    ;; "NodeWaiting"
    ;; "abs"
    ;; "arrayMaxSize"
    ;; "arraySize"
    ;; "ceil"
    ;; "floor"
    ;; "isKnown"
    ;; "max"
    ;; "min"
    ;; "mod"
    ;; "real_to_int"
    ;; "round"
    ;; "strlen"
    ;; "trunc"
    ))

;;; Statement keywords followed by a parenthesis expression that
;;; nevertheless contains a list separated with `;' and not `,'.
(c-lang-defconst c-paren-stmt-kwds
  plexil
  '("for"))

;;; The keyword(s) which introduce a "case" like construct.
;;; This construct is "<keyword> <expression> :".
(c-lang-defconst c-case-kwds
  plexil
  nil)

;;; Keywords besides constants and operators that start primary expressions.
(c-lang-defconst c-primary-expr-kwds
  plexil
  '("Parent" "Self"))

;;; List of all assignment operators.
(c-lang-defconst c-assignment-operators
  plexil
  '("="))

;;; N.B. Order of this list is the reverse of the hierarchy in Plexil.g.
(c-lang-defconst c-operators
  plexil
  `(
    ;; Preprocessor. Keep it simple for now.
    (prefix "#")
    (left-assoc "##")

    ;; Member references.
    (left-assoc ".")

    ;; Postfix (array ref, fn call)
    (postfix "[" "]" "(" ")")

    ;; Unary.
    (prefix "+" "-" "!" "NOT")

    ;; Multiplicative
    (left-assoc "*" "/" "%" "mod")

    ;; Additive.
    (left-assoc "+" "-")

    ;; Relational.
    (left-assoc "<" ">" "<=" ">=")

    ;; Equality.
    (left-assoc "==" "!=")

    ;; Bitwise and (NYI)
    ;; Bitwise xor (NYI)
    ;; Bitwise or  (NYI)

    ;; Logical AND.
    (left-assoc "&&" "AND")

    ;; Logical XOR.
    (left-assoc "XOR")

    ;; Logical OR.
    (left-assoc "||" "OR")

    ;; Conditional. (NYI)

    ;; Assignment.
    (right-assoc ,@(c-lang-const c-assignment-operators))

    ;; Sequence (NYI)
    ))

;;; Builds the syntax table for Standard PLEXIL.
(c-lang-defconst c-make-mode-syntax-table
  plexil
  (lambda ()
    (let ((table (make-syntax-table)))
      (c-populate-syntax-table table)
      ;; " is a string quote character. (??)
      (modify-syntax-entry ?\" "\"" table)
      table)
    ))

;;; Keywords that might be followed by a label identifier. (?)
(c-lang-defconst c-before-label-kwds
  plexil
  nil)

;;; Keywords for constants.
(c-lang-defconst c-constant-kwds
  plexil
  '(
    ;; Node state values
    "EXECUTING"
    "FAILING"
    "FINISHED"
    "FINISHING"
    "INACTIVE"
    "ITERATION_ENDED"
    "WAITING"

    ;; Node outcome values
    "FAILURE"
    "INTERRUPTED"
    "SKIPPED"
    "SUCCESS"

    ;; Node failure values
    "EXITED"
    "INVARIANT_CONDITION_FAILED"
    "PARENT_EXITED"
    "PARENT_FAILED"
    "POST_CONDITION_FAILED"
    "PRE_CONDITION_FAILED"

    ;; Command handle values
    "COMMAND_ACCEPTED"
    "COMMAND_DENIED"
    "COMMAND_FAILED"
    "COMMAND_RCVD_BY_SYSTEM"
    "COMMAND_SENT_TO_SYSTEM"
    "COMMAND_SUCCESS"
    
    ;; Boolean literals
    "false" "true"
    ))

;;;
;;; Font-lock hackery
;;;

(c-override-default-keywords 'plexil-font-lock-keywords)

(defconst plexil-font-lock-keywords-1 (c-lang-const c-matchers-1 plexil)
  "Minimal font locking for PLEXIL mode.
Fontifies only preprocessor directives (in addition to the
syntactic fontification of strings and comments).")

(defconst plexil-font-lock-keywords-2 (c-lang-const c-matchers-2 plexil)
  "Fast normal font locking for PLEXIL mode.
In addition to `plexil-font-lock-keywords-1', this adds fontification of
keywords, simple types, declarations that are easy to recognize, the
user defined types on `plexil-font-lock-extra-types', and the doc comment
styles specified by `c-doc-comment-style'.")

(defconst plexil-font-lock-keywords-3 (c-lang-const c-matchers-3 plexil)
  "Accurate normal font locking for PLEXIL mode.
Like the variable `plexil-font-lock-keywords-2' but detects declarations in a more
accurate way that works in most cases for arbitrary types without the
need for `plexil-font-lock-extra-types'.")

(defvar plexil-font-lock-keywords plexil-font-lock-keywords-3
  "Default expressions to highlight in PLEXIL mode.")

(defun plexil-font-lock-keywords-2 ()
  (c-compose-keywords-list plexil-font-lock-keywords-2))
(defun plexil-font-lock-keywords-3 ()
  (c-compose-keywords-list plexil-font-lock-keywords-3))
(defun plexil-font-lock-keywords ()
  (c-compose-keywords-list plexil-font-lock-keywords))

;; Correct some indentation issues with c-indent-line-or-region.
;; (defun set-newline-and-indent-node ()

;;   (if (looking-at "[ \t]*[a-z0-9A-Z_]+[:]?[ ]+[a-z0-9A-Z]*")
;;       (progn (local-set-key (kbd ":") '(":"))
;;          (local-set-key (kbd "{") '("{"))
;;          (local-set-key (kbd "}") '("}"))
;;          (local-set-key (kbd ")") '(")"))
;;          (local-set-key (kbd "(") '("("))
;;          (local-set-key (kbd ";") '(";"))
;;          ))
;;   (if (looking-at "[...]*{")
;;       ;;(local-set-key (kbd TAB) '(DEL))
;;     (local-set-key (kbd "RET") '("RET"))
;;     (local-set-key (kbd ":") '(":"))
;;     (local-set-key (kbd "{") '("{")))

;;   ;; Supposed to stop TAB from indenting on an empty line in a new node. Not currently working.
;;   (if (looking-at "[...]*{[ \t]*[\n]+")
;;       (local-set-key (kbd "TAB") '(nil)))

;;   )

;; (add-hook 'plexil-mode-hook 'set-newline-and-indent-node)

;;;
;;; Menus
;;;

;;; See imenu.el 
(defvar cc-imenu-plexil-generic-expression
  cc-imenu-c++-generic-expression
  "Imenu generic expression for PLEXIL mode.  See `imenu-generic-expression'.")

;;;
;;; Wrap it up
;;;

;;; Add to auto mode alist
;;; Comment stolen from cc-mode.el:
;; In XEmacs >= 21.5 modes should add their own entries to
;; `auto-mode-alist'.  The comment form of autoload is used to avoid
;; doing this on load.  That since `add-to-list' prepends the value
;; which could cause it to clobber user settings.  Later emacsen have
;; an append option, but it's not safe to use.
;;;###autoload (add-to-list 'auto-mode-alist '("\\.ple\\'" . plexil-mode))
;;;###autoload (add-to-list 'auto-mode-alist '("\\.plp\\'" . plexil-mode))

(defcustom plexil-mode-hook nil
  "Normal hook run when entering 'plexil-mode'.
See 'run-hooks'."
  :type 'hook
  :group 'c
  )

;;; Define mode, derive from c-mode. Set syntax highlighting and indentation.
;;;###autoload
(define-derived-mode plexil-mode c-mode "PLEXIL"
  "Major mode for editing PLEXIL plans.

The hook `c-mode-common-hook' is run with no args at mode
initialization, then `plexil-mode-hook'.

Key bindings:
\\{plexil-mode-map}"

  ;; Body patterned after those in cc-mode.el

  ;; :after-hook runs after all the mode hooks
  :after-hook (progn
                (c-make-noise-macro-regexps)
                (c-make-macro-with-semi-re)
                (c-update-modeline)
                )

  ;; Rest of body runs before mode hooks
  (c-initialize-cc-mode t)
  (setq abbrev-mode t)
  (c-init-language-vars plexil-mode)
  (c-common-init 'plexil-mode)
  (easy-menu-add c-plexil-menu)
  (cc-imenu-init cc-imenu-plexil-generic-expression)
  (c-run-mode-hooks 'c-mode-common-hook)
  )

;; Set C style to a more appropriate indentation style for PLEXIL
;; and a proper offset.
(c-add-style "plexil"
             ;; Derived from ellemtel style
             '((c-basic-offset . 2)
               (c-comment-only-line-offset . 0)
               (c-hanging-braces-alist . ((substatement-open before after)
                                          (arglist-cont-nonempty)))
               (c-offsets-alist . ((topmost-intro        . 0)
			                       (substatement         . +)
			                       (substatement-open    . 0)
			                       (case-label           . +)
			                       (access-label         . -)
			                       (inclass              . +)
			                       (inline-open          . 0)))
               )
             nil) ;; don't set it when loading, please!

;;; Main mode menu
(easy-menu-define c-plexil-menu plexil-mode-map "PLEXIL Mode Commands"
  (cons "PLEXIL" (c-lang-const c-mode-menu plexil)))

(provide 'plexil-mode)
