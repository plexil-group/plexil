;;; Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

(defun init-plexilisp (&optional compile)
  (interactive)
  (let* ((plexildir (format "%s/src/plexilisp" (getenv "PLEXIL_HOME")))
         (source-file (format "%s/plexil.el" plexildir))
         (compiled-file (format "%s/plexil.elc" plexildir))
         (max-specpdl-size 2700))
    (if compile (byte-compile-file source-file))
    (if (file-exists-p compiled-file)
        (load-file compiled-file)
      (progn
        (message "Can't find compiled Plexilisp file.  Use M-x compile-plexilisp.")
        (sleep-for 2)
        (load-file source-file)))))

(defun compile-plexilisp ()
  (interactive)
  (init-plexilisp t))

(if (null (getenv "PLEXIL_HOME"))
    ;; Sometimes, Emacs doesn't see the environment variables defined by the user.
    ;; I haven't yet figured out why.
    (message (concat "Error: PLEXIL_HOME undefined in Emacs. "
                     "Try defining it in your .emacs file. "
                     "See your plexil/src/plexilisp/emacs.el for instructions. "))
  (init-plexilisp))

;;; If you got the error message in the form above, put the following in
;;; your .emacs file, just before the line that loads this file:
;;;     (setenv "PLEXIL_HOME" "/home/fred/plexil")
;;; replacing the pathname appropriately -- it should be that of your
;;; 'plexil' installation directory.  Do not use the '~' character.



