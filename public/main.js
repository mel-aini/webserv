const buttons = document.querySelectorAll(".expand");

buttons.forEach((button) => {
	button.addEventListener("click", () => {
		const scope = button.closest(".scope");

		scope.classList.toggle('active');
	})
})

document.getElementById('uploadForm').addEventListener('submit', function (event) {
	event.preventDefault();
	
	var formData = new FormData(this);
	fetch('/', {
	  method: 'POST',
	  body: formData
	})
	.then(response => {
		alert("File uploaded successfully");
	})
	.catch(error => {
	  // Handle errors
	  console.error('Error uploading file:', error);
	});
  });
