function uploadFile() {
  const file = document.getElementById("file-input").files[0];
  if (!file) return;
  fetch("/uploads/" + file.name, {
    method: "POST",
    headers: { "Content-Type": file.type },
    body: file,
  })
    .then(
      (r) =>
        (document.getElementById("upload-result").textContent =
          r.status + " " + r.statusText),
    )
    .catch((e) => (document.getElementById("upload-result").textContent = e));
}

function loadFiles() {
  fetch("/uploads/")
    .then((r) => r.text())
    .then((html) => {
      const parser = new DOMParser();
      const doc = parser.parseFromString(html, "text/html");
      const links = [...doc.querySelectorAll("a")];
      const files = links
        .map((a) => a.getAttribute("href"))
        .filter((h) => h && !h.startsWith("?") && h !== "../" && h !== "/");

      const list = document.getElementById("file-list");
      if (files.length === 0) {
        list.textContent = "No files uploaded yet.";
        return;
      }
      list.innerHTML = files
        .map(
          (f) =>
            `<div class="file-row">
			  <span>${f}</span>
			  <button onclick="deleteFile('${f}')">DELETE</button>
			</div>`,
        )
        .join("");
    });
}

function deleteFile(filename) {
  fetch("/uploads/" + filename, { method: "DELETE" })
    .then((r) => {
      document.getElementById("del-result").textContent =
        filename + " → " + r.status + " " + r.statusText;
      loadFiles();
    })
    .catch((e) => (document.getElementById("del-result").textContent = e));
}

function demoRedirect() {
  const out = document.getElementById("redirect-result");
  out.textContent =
    "Address /old was redirected (301).\nRequesting redirected address /new-location ...";
  fetch("/old", { redirect: "manual" }).then(
    (r) => (out.textContent += "\n/old returned: " + r.status + " " + r.type),
  );
  setTimeout(() => {
    fetch("/new-location")
      .then((r) => r.text())
      .then((html) => {
        document.open();
        document.write(html);
        document.close();
      })
      .catch((e) => (out.textContent += "\n\nError: " + e));
  }, 2000);
}

loadFiles();
