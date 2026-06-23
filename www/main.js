/* ============================================
   WEBSERV DASHBOARD — Real Network Logic
   ============================================ */

document.addEventListener('DOMContentLoaded', () => {
  initNavbar();
  initUploadPage();
  initGalleryPage();
  initCgiPage();
});

/* ---------- Navbar Response ---------- */
function initNavbar() {
  const hamburger = document.getElementById('navHamburger');
  const navLinks = document.getElementById('navLinks');

  if (hamburger && navLinks) {
    hamburger.addEventListener('click', () => {
      navLinks.classList.toggle('open');
      hamburger.classList.toggle('active');
    });
  }
}

/* ---------- Upload Page Features (REAL POST INTEGRATION) ---------- */
function initUploadPage() {
  const dropzone = document.getElementById('uploadDropzone');
  const fileInput = document.getElementById('fileInput');
  const btnSelectFiles = document.getElementById('btnSelectFiles');
  const fileTable = document.getElementById('fileTable');

  if (!dropzone) return;

  if (btnSelectFiles && fileInput) {
    btnSelectFiles.addEventListener('click', (e) => {
      e.stopPropagation();
      fileInput.click();
    });

    fileInput.addEventListener('change', () => {
      handleFiles(fileInput.files);
    });
  }

  dropzone.addEventListener('dragover', (e) => {
    e.preventDefault();
    dropzone.classList.add('drag-over');
  });

  dropzone.addEventListener('dragleave', () => {
    dropzone.classList.remove('drag-over');
  });

  dropzone.addEventListener('drop', (e) => {
    e.preventDefault();
    dropzone.classList.remove('drag-over');
    handleFiles(e.dataTransfer.files);
  });

  dropzone.addEventListener('click', () => {
    if (fileInput) fileInput.click();
  });

  // REAL LOGIC: Sends data to your C++ server
  function handleFiles(files) {
    if (!files.length) return;
    
    const streamPlaceholder = document.querySelector('.stream-placeholder');
    if (!streamPlaceholder) return;

    // Set UI to transmission mode
    streamPlaceholder.innerHTML = `
      <div class="flex flex-col items-center gap-8 w-full">
        <div class="badge badge-purple" style="width: 100%; justify-content: center; animation: pulse 1.5s infinite;">TRANSFERENCE IN PROGRESS</div>
        <span style="font-size:11px; color:var(--text-secondary); text-align:center;">
          Uploading ${files.length} file(s) natively...
        </span>
      </div>
    `;

    // Process file elements asynchronously 
    Array.from(files).forEach(file => {
      const formData = new FormData();
      formData.append("file", file);

      // Make a true network POST to your server configuration block route
      fetch('/upload', {
        method: 'POST',
        body: formData
      })
      .then(async (response) => {
        if (!response.ok) throw new Error(`Server returned ${response.status}`);
        return response.text();
      })
      .then(() => {
        // Success UI rendering
        streamPlaceholder.innerHTML = `
          <div class="flex flex-col items-center gap-8 w-full">
            <svg viewBox="0 0 24 24" fill="none" stroke="var(--accent-green)" stroke-width="2" width="24" height="24"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14.01 9 11.01"/></svg>
            <span style="font-size:11px; color:var(--accent-green); font-weight:600;">"${file.name}" Ingested!</span>
          </div>
        `;

        // Inject row directly into the visual structural table
        appendFileToTable(file);
      })
      .catch((error) => {
        // Error handling visual loop
        streamPlaceholder.innerHTML = `
          <div class="flex flex-col items-center gap-8 w-full">
            <span style="font-size:11px; color:#ef4444; font-weight:600;">Transmission Error</span>
            <span style="font-size:10px; color:var(--text-secondary);">${error.message}</span>
          </div>
        `;
      })
      .finally(() => {
        // Reset placeholder back to baseline after timeout delay
        setTimeout(() => {
          streamPlaceholder.innerHTML = `
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="17 8 12 3 7 8"/><line x1="12" y1="3" x2="12" y2="15"/></svg>
            <span>No active transmissions</span>
          `;
        }, 4000);
      });
    });
  }

  function appendFileToTable(file) {
    const tbody = fileTable.querySelector('tbody');
    const row = document.createElement('tr');
    const ext = file.name.split('.').pop().toLowerCase();
    let formatBadge = 'TXT';
    let iconClass = 'txt';

    if (['conf', 'cfg'].includes(ext)) { formatBadge = 'CONF'; iconClass = 'conf'; }
    else if (['png', 'jpg', 'jpeg', 'webp', 'gif'].includes(ext)) { formatBadge = 'IMAGE'; iconClass = 'image'; }
    else if (['cgi', 'py', 'sh', 'pl'].includes(ext)) { formatBadge = 'CGI'; iconClass = 'cgi'; }
    else if (ext === 'pdf') { formatBadge = 'PDF'; iconClass = 'pdf'; }

    const formattedSize = file.size > 1024 * 1024 
      ? (file.size / (1024 * 1024)).toFixed(1) + ' MB'
      : (file.size / 1024).toFixed(1) + ' KB';

    const today = new Date().toISOString().slice(0, 16).replace('T', ' ');

    row.innerHTML = `
      <td>
        <div class="file-name-cell">
          <div class="file-icon ${iconClass}">
            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/></svg>
          </div>
          ${file.name}
        </div>
      </td>
      <td><span class="format-badge ${iconClass}">${formatBadge}</span></td>
      <td class="file-size">${formattedSize}</td>
      <td class="file-date">${today}</td>
      <td>
        <button class="file-delete-btn" data-filename="${file.name}" aria-label="Delete file">
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"/><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"/></svg>
        </button>
      </td>
    `;
    tbody.insertBefore(row, tbody.firstChild);
    bindDeleteBtn(row.querySelector('.file-delete-btn'));

    const countBadge = document.querySelector('.file-count');
    if (countBadge) {
      countBadge.textContent = `${tbody.querySelectorAll('tr').length} Total Objects`;
    }
  }

  // REAL LOGIC: Sends HTTP DELETE requests to server 
  function bindDeleteBtn(button) {
    button.addEventListener('click', (e) => {
      e.stopPropagation();
      const row = button.closest('tr');
      // Retrieve original explicit name (fallback to dynamic text string parsing if attribute missing)
      const filename = button.getAttribute('data-filename') || row.querySelector('.file-name-cell').textContent.trim();

      if (!row) return;

      // Dispatch real HTTP DELETE standard package method
      fetch(`/upload/${filename}`, {
        method: 'DELETE'
      })
      .then(response => {
        if (!response.ok) throw new Error("Delete method disallowed by backend rules.");
        
        // Dynamic UI Removal Animation
        row.style.opacity = '0';
        row.style.transform = 'translateX(20px)';
        row.style.transition = 'all 0.3s ease';
        setTimeout(() => {
          row.remove();
          const countBadge = document.querySelector('.file-count');
          if (countBadge) {
            countBadge.textContent = `${fileTable.querySelector('tbody').querySelectorAll('tr').length} Total Objects`;
          }
        }, 300);
      })
      .catch(err => {
        alert(`Failed to delete object from server: ${err.message}`);
      });
    });
  }

  document.querySelectorAll('.file-delete-btn').forEach(btn => bindDeleteBtn(btn));
}

/* ---------- Gallery Page Filtering & Sorting ---------- */
function initGalleryPage() {
  const categoryItems = document.querySelectorAll('.category-item');
  const fileGrid = document.getElementById('fileGrid');
  const searchInput = document.getElementById('gallerySearchInput');
  const sortSelect = document.getElementById('gallerySort');
  const viewGridBtn = document.getElementById('viewGridBtn');
  const viewListBtn = document.getElementById('viewListBtn');

  if (!fileGrid) return;

  if (viewGridBtn && viewListBtn) {
    viewGridBtn.addEventListener('click', () => {
      viewGridBtn.classList.add('active');
      viewListBtn.classList.remove('active');
      fileGrid.style.gridTemplateColumns = 'repeat(4, 1fr)';
      document.querySelectorAll('.file-card-preview').forEach(el => el.style.height = '160px');
    });

    viewListBtn.addEventListener('click', () => {
      viewListBtn.classList.add('active');
      viewGridBtn.classList.remove('active');
      fileGrid.style.gridTemplateColumns = '1fr';
      document.querySelectorAll('.file-card-preview').forEach(el => el.style.height = '60px');
    });
  }

  categoryItems.forEach(item => {
    item.addEventListener('click', () => {
      categoryItems.forEach(i => i.classList.remove('active'));
      item.classList.add('active');
      const targetCategory = item.getAttribute('data-category');
      filterFiles(targetCategory, searchInput ? searchInput.value : '');
    });
  });

  if (searchInput) {
    searchInput.addEventListener('input', () => {
      const activeCategoryItem = document.querySelector('.category-item.active');
      const activeCategory = activeCategoryItem ? activeCategoryItem.getAttribute('data-category') : 'all';
      filterFiles(activeCategory, searchInput.value);
    });
  }

  function filterFiles(category, searchVal) {
    const cards = fileGrid.querySelectorAll('.file-card');
    const cleanSearch = searchVal.trim().toLowerCase();

    cards.forEach(card => {
      const type = card.getAttribute('data-type');
      const name = card.getAttribute('data-name').toLowerCase();
      const matchesCategory = (category === 'all' || type === category);
      const matchesSearch = (cleanSearch === '' || name.includes(cleanSearch));
      card.style.display = (matchesCategory && matchesSearch) ? 'block' : 'none';
    });
  }

  if (sortSelect) {
    sortSelect.addEventListener('change', () => {
      const cards = Array.from(fileGrid.querySelectorAll('.file-card'));
      const sortVal = sortSelect.value;

      cards.sort((a, b) => {
        if (sortVal === 'name') return a.getAttribute('data-name').localeCompare(b.getAttribute('data-name'));
        if (sortVal === 'newest') return new Date(b.getAttribute('data-date')) - new Date(a.getAttribute('data-date'));
        if (sortVal === 'oldest') return new Date(a.getAttribute('data-date')) - new Date(b.getAttribute('data-date'));
        return 0;
      });
      cards.forEach(card => fileGrid.appendChild(card));
    });
  }
}

/* ---------- CGI Terminal Emulation (REAL ROUTE EXECUTION) ---------- */
function initCgiPage() {
  const terminalBody = document.getElementById('terminalBody');
  const commandField = document.getElementById('commandField');
  const btnExecute = document.getElementById('btnExecute');
  const btnClearConsole = document.getElementById('btnClearConsole');
  const scriptPills = document.querySelectorAll('.script-pill');

  if (!terminalBody) return;

  scriptPills.forEach(pill => {
    pill.addEventListener('click', () => {
      scriptPills.forEach(p => p.classList.remove('active'));
      pill.classList.add('active');
      const scriptName = pill.getAttribute('data-script');
      if (commandField) {
        commandField.value = `./${scriptName}`;
        commandField.focus();
      }
    });
  });

  if (btnClearConsole) {
    btnClearConsole.addEventListener('click', () => {
      terminalBody.innerHTML = `
        <div class="terminal-line">
          <span class="terminal-timestamp">${getCurrentTime()}</span>
          <span class="terminal-text info">Console cleared. Shell standing by.</span>
        </div>
      `;
    });
  }

  if (btnExecute && commandField) {
    btnExecute.addEventListener('click', () => { executeCommand(commandField.value); });
    commandField.addEventListener('keypress', (e) => {
      if (e.key === 'Enter') executeCommand(commandField.value);
    });
  }

  // REAL LOGIC: Routes query text payload strings to your actual CGI gateway engine
  function executeCommand(cmdText) {
    const cleanCmd = cmdText.trim();
    if (!cleanCmd) return;

    appendTerminalLine(cleanCmd, 'command', 'webserv@admin:~$ ');

    if (cleanCmd.toLowerCase() === 'clear') {
      terminalBody.innerHTML = '';
      commandField.value = '';
      return;
    }

    // Isolate pure script parsing identifiers
    let targetScriptPath = cleanCmd.replace('./', '');
    
    // Fallback handlers for common default baseline shell requests
    if (cleanCmd.toLowerCase() === 'help') {
      appendTerminalLine('Available routing scripts:\n  - ./hello_world.py\n  - ./server_info.sh\n  - ./db_query.py', 'info');
      commandField.value = '';
      return;
    }
    if (cleanCmd.toLowerCase() === 'status') {
      appendTerminalLine('STATUS: RUNNING THROUGH INTERCEPT SOCK LOOP\nSTANDARDS: C++98 ENGINE COMPLIANT', 'success');
      commandField.value = '';
      return;
    }

    // Process actual network query down to your script pipeline paths via GET requests
    fetch(`/cgi-bin/${targetScriptPath}`)
    .then(async (response) => {
      appendTerminalLine(`Status returned: ${response.status}`, 'info');
      return response.text();
    })
    .then((rawOutput) => {
      // Split lines cleanly to feed output down to the terminal terminal interface window
      const outputLines = rawOutput.split('\n');
      outputLines.forEach(line => {
        if (line.trim()) {
          appendTerminalLine(line, 'output');
        }
      });
    })
    .catch((error) => {
      appendTerminalLine(`webserv-cgi-error execution failure: ${error.message}`, 'danger');
    })
    .finally(() => {
      terminalBody.scrollTop = terminalBody.scrollHeight;
    });

    commandField.value = '';
  }

  function appendTerminalLine(text, cssClass, promptText = '') {
    const line = document.createElement('div');
    line.className = 'terminal-line';
    
    const timestamp = document.createElement('span');
    timestamp.className = 'terminal-timestamp';
    timestamp.textContent = getCurrentTime();

    const textSpan = document.createElement('span');
    textSpan.className = `terminal-text ${cssClass}`;
    
    const escaped = text
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#039;');
      
    textSpan.innerHTML = promptText ? `<strong>${promptText}</strong>${escaped}` : escaped;

    line.appendChild(timestamp);
    line.appendChild(textSpan);
    terminalBody.appendChild(line);
  }

  function getCurrentTime() {
    return new Date().toTimeString().split(' ')[0];
  }
}